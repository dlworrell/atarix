"""Signing and concurrency regressions for generated-file publishing."""
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest import mock

spec=importlib.util.spec_from_file_location("publisher",Path(__file__).resolve().parents[1]/".github/scripts/publish_signed_pr.py")
publisher=importlib.util.module_from_spec(spec)
spec.loader.exec_module(publisher)
OID="f"*40
BASE="a"*40
class PublisherTest(unittest.TestCase):
    def execute(self,verification=None,parents=None,old=None,lease_error=False,event="workflow_dispatch",default="main",pr_error=None,env=None,extra=()):
        calls=[]
        def fake(*args,stdin=None):
            calls.append((args,json.loads(stdin) if stdin else None))
            if args[:3]==("gh","pr","create"):
                if pr_error: raise RuntimeError(pr_error)
                return "https://example.invalid/pr/1"
            if args[:3]==("gh","pr","list"): return "[]"
            endpoint=args[2]
            if endpoint=="repos/o/r": return json.dumps({"node_id":"REPO","default_branch":default})
            if "matching-refs" in endpoint:
                return json.dumps([] if old is None else [{"ref":"refs/heads/automation/x","object":{"sha":old}}])
            if endpoint=="graphql":
                payload=json.loads(stdin)
                if "UpdateRefsInput" in payload["query"]:
                    if lease_error: return json.dumps({"errors":[{"message":"stale beforeOid"}]})
                    return '{"data":{"updateRefs":{"clientMutationId":null}}}'
                return json.dumps({"data":{"createCommitOnBranch":{"commit":{"oid":OID,"url":"u"}}}})
            if endpoint.endswith("/commits/"+OID):
                return json.dumps({"sha":OID,"commit":{"verification":verification or {}},
                                   "parents":[{"sha":p} for p in (parents if parents is not None else [BASE])]})
            return "{}"
        with mock.patch.dict(os.environ,{"GITHUB_REPOSITORY":"o/r","GITHUB_EVENT_NAME":event,**(env or {})}), \
             mock.patch.object(publisher,"changes",return_value=([{"path":"x","contents":"eA=="}],[])), \
             mock.patch.object(publisher,"run",side_effect=fake):
            result=publisher.main(["--branch","automation/x","--title","fix: x","--base","main","--base-sha",BASE,*extra,"x"])
        return result,calls
    def test_verified_signature_allows_pull_request(self):
        result,calls=self.execute({"verified":True,"reason":"valid"})
        self.assertEqual(result,0)
        self.assertTrue(any(a[:3]==("gh","pr","create") for a,_ in calls))
    def test_unsigned_invalid_missing_and_non_boolean_fail(self):
        for value in ({},{"verified":False,"reason":"unsigned"},{"verified":True,"reason":"unknown_key"},{"verified":1,"reason":"valid"}):
            with self.subTest(value=value), self.assertRaisesRegex(RuntimeError,"did not verify"):
                self.execute(value)
    def test_wrong_parent_is_rejected(self):
        with self.assertRaisesRegex(RuntimeError,"did not verify"):
            self.execute({"verified":True,"reason":"valid"},parents=["b"*40])
    def test_force_update_is_bound_to_observed_head(self):
        result,calls=self.execute({"verified":True,"reason":"valid"},old="b"*40)
        payload=next(p for a,p in calls if p and "UpdateRefsInput" in p["query"])
        self.assertEqual(payload["variables"]["input"]["refUpdates"],[
            {"name":"refs/heads/automation/x","beforeOid":"b"*40,"afterOid":BASE,"force":True}])
    def test_stale_lease_aborts_before_signing(self):
        with self.assertRaisesRegex(RuntimeError,"refused.*stale beforeOid"):
            self.execute({"verified":True,"reason":"valid"},old="b"*40,lease_error=True)
    def test_pull_request_target_and_default_branch_are_rejected(self):
        with self.assertRaisesRegex(RuntimeError,"pull_request_target"):
            self.execute(event="pull_request_target")
        with self.assertRaisesRegex(RuntimeError,"Default branch"):
            self.execute(default="automation/x")
    def test_command_error_retains_stderr_and_failure(self):
        error=subprocess.CalledProcessError(1,("gh","pr","create"),stderr="GraphQL: specific policy refusal\n")
        with mock.patch.object(publisher.subprocess,"run",side_effect=error):
            with self.assertRaisesRegex(RuntimeError,"exit 1.*specific policy refusal"):
                publisher.run("gh","pr","create")
    def test_disabled_actions_policy_identifies_the_retained_verified_commit(self):
        error="GitHub Actions is not permitted to create or approve pull requests (createPullRequest)"
        with self.assertRaises(RuntimeError) as failure:
            self.execute({"verified":True,"reason":"valid"},pr_error=error)
        message=str(failure.exception)
        self.assertIn(f"Verified commit {OID} is retained on automation/x",message)
        self.assertIn("repository Actions policy",message)
        self.assertIn(error,message)
    def test_other_pr_errors_remain_failures(self):
        with self.assertRaisesRegex(RuntimeError,"unrelated service error"):
            self.execute({"verified":True,"reason":"valid"},pr_error="unrelated service error")
    def test_command_errors_never_carry_credentials(self):
        secret="ghs_"+"S"*36
        pat="github_pat_"+"P"*40
        error=subprocess.CalledProcessError(1,("gh","api"),stderr=f"Bad credentials {secret} {pat} env-value-123\n")
        with mock.patch.dict(os.environ,{"GH_TOKEN":"env-value-123"}), \
             mock.patch.object(publisher.subprocess,"run",side_effect=error):
            with self.assertRaises(RuntimeError) as failure:
                publisher.run("gh","api","user")
        message=str(failure.exception)
        self.assertIn("Bad credentials",message)
        for value in (secret,pat,"env-value-123"):
            self.assertNotIn(value,message)
        self.assertIsNone(failure.exception.__cause__)
    def test_graphql_refusal_reports_redacted_detail(self):
        secret="ghs_"+"T"*36
        with mock.patch.object(publisher,"run",return_value=json.dumps({"errors":[{"message":f"denied {secret}"}]})):
            with self.assertRaisesRegex(RuntimeError,"refused.*denied \\*\\*\\*") as failure:
                publisher.graphql({"query":"q"})
        self.assertNotIn(secret,str(failure.exception))
    def test_policy_refusal_writes_compare_link_to_step_summary(self):
        with tempfile.TemporaryDirectory() as tmp:
            summary=Path(tmp)/"summary.md"
            error="GitHub Actions is not permitted to create or approve pull requests (createPullRequest)"
            with self.assertRaises(RuntimeError):
                self.execute({"verified":True,"reason":"valid"},pr_error=error,
                             env={"GITHUB_STEP_SUMMARY":str(summary),"GITHUB_SERVER_URL":"https://github.com"})
            text=summary.read_text()
        self.assertIn(f"Verified commit `{OID}` is on `automation/x`",text)
        self.assertIn("https://github.com/o/r/compare/main...automation/x?expand=1",text)
    def test_unrelated_pr_errors_write_no_summary(self):
        with tempfile.TemporaryDirectory() as tmp:
            summary=Path(tmp)/"summary.md"
            with self.assertRaises(RuntimeError):
                self.execute({"verified":True,"reason":"valid"},pr_error="unrelated service error",
                             env={"GITHUB_STEP_SUMMARY":str(summary)})
            self.assertFalse(summary.exists())
    def test_ring0_opener_publishes_verified_commit_without_touching_pull_requests(self):
        with tempfile.TemporaryDirectory() as tmp:
            summary=Path(tmp)/"summary.md"
            result,calls=self.execute({"verified":True,"reason":"valid"},extra=("--pr-opener","ring-0"),
                                      env={"GITHUB_STEP_SUMMARY":str(summary)})
            text=summary.read_text()
        self.assertEqual(result,0)
        self.assertFalse(any(a[:2]==("gh","pr") for a,_ in calls))
        self.assertTrue(any(p and "CreateCommitOnBranchInput" in p["query"] for _,p in calls))
        self.assertIn(f"Verified commit `{OID}` is on `automation/x`",text)
    def test_ring0_opener_still_requires_a_verified_commit(self):
        with self.assertRaisesRegex(RuntimeError,"did not verify"):
            self.execute({"verified":False,"reason":"unsigned"},extra=("--pr-opener","ring-0"))

if __name__=="__main__":
    unittest.main()
