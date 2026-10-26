#!/usr/bin/env python3
"""Publish generated files through verified GitHub commits and leased automation refs.

A Signed-off-by trailer is not a cryptographic signature. GitHub verification,
the exact commit identity and its parent are checked before reporting success.
Only the workflow-owned automation branch is updated, using beforeOid as a lease.
"""
from __future__ import annotations
import argparse
import base64
import json
import os
import re
import subprocess
from pathlib import Path

MUTATION = "mutation($input:CreateCommitOnBranchInput!){createCommitOnBranch(input:$input){commit{oid url}}}"
REF_MUTATION = "mutation($input:UpdateRefsInput!){updateRefs(input:$input){clientMutationId}}"

TOKEN_PATTERN = re.compile(r"\b(?:gh[pousr]_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{20,})\b")
POLICY_REJECTION = "not permitted to create or approve pull requests"

def redact(text: str) -> str:
    """Remove credential values from text that may reach logs or summaries."""
    for name in ("GH_TOKEN","GITHUB_TOKEN"):
        value=os.environ.get(name)
        if value:
            text=text.replace(value,"***")
    return TOKEN_PATTERN.sub("***",text)

def run(*args: str, stdin: str | None = None) -> str:
    try:
        return subprocess.run(args,input=stdin,check=True,capture_output=True,text=True).stdout
    except subprocess.CalledProcessError as error:
        detail=redact((error.stderr or error.stdout or "no command output").strip())
        raise RuntimeError(f"{' '.join(args[:2])} failed (exit {error.returncode}): {detail}") from None

def summarize(text: str) -> None:
    path=os.environ.get("GITHUB_STEP_SUMMARY")
    if path:
        with open(path,"a",encoding="utf-8") as handle:
            handle.write(redact(text)+"\n")

def changes(base_sha: str, paths: list[str]) -> tuple[list[dict],list[dict]]:
    run("git","add","-A","--",*paths)
    fields=run("git","diff","--cached","--name-status","--no-renames","-z",base_sha,"--",*paths).split("\0")
    additions,deletions=[],[]
    for code,path in zip(fields[0::2],fields[1::2]):
        if code=="D":
            deletions.append({"path":path})
        else:
            additions.append({"path":path,"contents":base64.b64encode(Path(path).read_bytes()).decode("ascii")})
    return additions,deletions

def graphql(payload: dict) -> dict:
    result=json.loads(run("gh","api","graphql","--input","-",stdin=json.dumps(payload)))
    if not isinstance(result,dict) or result.get("errors"):
        messages="; ".join(str(e.get("message","")) for e in result.get("errors",[]) if isinstance(e,dict)) \
            if isinstance(result,dict) else ""
        raise RuntimeError("GitHub refused the leased ref update or commit mutation: "+redact(messages or "no detail"))
    return result

def verify_commit(repo: str, oid: str, parent: str) -> None:
    observed=json.loads(run("gh","api",f"repos/{repo}/commits/{oid}"))
    verification=observed.get("commit",{}).get("verification",{})
    if (observed.get("sha")!=oid or verification.get("verified") is not True
            or verification.get("reason")!="valid"
            or [p.get("sha") for p in observed.get("parents",[])]!=[parent]):
        raise RuntimeError(f"GitHub did not verify the exact replacement commit {oid}")

def main(argv: list[str] | None = None) -> int:
    parser=argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--branch",required=True)
    parser.add_argument("--title",required=True)
    parser.add_argument("--body",default="")
    parser.add_argument("--base",default=os.environ.get("GITHUB_REF_NAME","main"))
    parser.add_argument("--base-sha",default=os.environ.get("GITHUB_SHA"))
    parser.add_argument("--pr-opener",choices=("self","ring-0"),default="self",
                        help="ring-0: leave opening the pull request to the catylist-ring-0 sweep (Catylist ADR-0012)")
    parser.add_argument("paths",nargs="+")
    args=parser.parse_args(argv)
    repo=os.environ["GITHUB_REPOSITORY"]
    if os.environ.get("GITHUB_EVENT_NAME")=="pull_request_target":
        raise RuntimeError("Refusing publishing from pull_request_target")
    if not args.base_sha:
        parser.error("--base-sha or GITHUB_SHA is required")
    if args.branch==args.base or not args.branch.startswith("automation/"):
        parser.error("An automation branch distinct from the base is required")
    additions,deletions=changes(args.base_sha,args.paths)
    if not additions and not deletions:
        print("No changes to publish.")
        return 0
    metadata=json.loads(run("gh","api",f"repos/{repo}"))
    if args.branch==metadata["default_branch"]:
        raise RuntimeError("Default branch writes are forbidden")
    ref=f"refs/heads/{args.branch}"
    refs=json.loads(run("gh","api",f"repos/{repo}/git/matching-refs/heads/{args.branch}"))
    head=next((r["object"]["sha"] for r in refs if r["ref"]==ref),None)
    if head is None:
        run("gh","api","-X","POST",f"repos/{repo}/git/refs","-f",f"ref={ref}","-f",f"sha={args.base_sha}")
    elif head!=args.base_sha:
        graphql({"query":REF_MUTATION,"variables":{"input":{
            "repositoryId":metadata["node_id"],"refUpdates":[{
                "name":ref,"beforeOid":head,"afterOid":args.base_sha,"force":True}]}}})
    result=graphql({"query":MUTATION,"variables":{"input":{
        "branch":{"repositoryNameWithOwner":repo,"branchName":args.branch},
        "expectedHeadOid":args.base_sha,"message":{"headline":args.title},
        "fileChanges":{"additions":additions,"deletions":deletions}}}})
    commit=result["data"]["createCommitOnBranch"]["commit"]
    verify_commit(repo,commit["oid"],args.base_sha)
    print(f"Verified commit {commit['oid']} on {args.branch}: {len(additions)} added or changed, {len(deletions)} deleted")
    if args.pr_opener=="ring-0":
        print(f"Pull request left to ring-0: it opens one for {args.branch} within its next sweep.")
        summarize(f"### Verified commit published\n\nVerified commit `{commit['oid']}` is on `{args.branch}`. "
                  "The catylist-ring-0 sweep opens the pull request within 30 minutes (Catylist ADR-0012).")
        return 0
    existing=json.loads(run("gh","pr","list","--repo",repo,"--head",args.branch,"--base",args.base,
                            "--state","open","--json","url") or "[]")
    if existing:
        print(f"Pull request already open: {existing[0]['url']}")
    else:
        try:
            url=run("gh","pr","create","--repo",repo,"--head",args.branch,"--base",args.base,
                    "--title",args.title,"--body",args.body or args.title).strip()
        except RuntimeError as error:
            if POLICY_REJECTION in str(error):
                server=os.environ.get("GITHUB_SERVER_URL","https://github.com")
                summarize(f"### Pull request not opened\n\nVerified commit `{commit['oid']}` is on "
                          f"`{args.branch}`. Repository Actions policy blocks PR creation by workflows. "
                          f"Open it from [the compare view]({server}/{repo}/compare/{args.base}...{args.branch}?expand=1).")
                raise RuntimeError(
                    f"Verified commit {commit['oid']} is retained on {args.branch}. "
                    "PR creation is blocked by the repository Actions policy: "
                    "'Allow GitHub Actions to create and approve pull requests' is disabled. "
                    "An authorized maintainer client can open the PR from this existing branch. "
                    "No replacement signature or new key is needed. " + str(error)
                ) from error
            raise
        print(f"Opened {url}")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
