# Generated report pull-request publication: rejection and recovery

Date: 2026-10-26. AI-assisted investigation and validation: Codex for @dlworrell.
Authority: Catylist ADR-0006 (verified commits and protected branches),
ADR-0012 (ring-0 opens automation PRs), ADR-0013 (reports target dev-daily).

## Observation and hypothesis

Runs [40248589256](https://github.com/dlworrell/atarix/actions/runs/40248589256)
and [40248589197](https://github.com/dlworrell/atarix/actions/runs/40248589197)
generated reports and verified their commits, then failed at `gh pr create`.
The original helper hid captured stderr. A missing token permission was a
hypothesis, not a confirmed cause. The original census job's complete log
actually includes `PullRequests: write`.

## Experiment and actual rejection

After error reporting was added in [PR #67](https://github.com/dlworrell/atarix/pull/67),
[run 40273574948](https://github.com/dlworrell/atarix/actions/runs/40273574948)
again verified its commit and recorded:

> GraphQL: GitHub Actions is not permitted to create or approve pull requests (createPullRequest)

That run also lists `PullRequests: write`. The rejection is the Actions
create/approve policy, not commit verification or an absent YAML permission.
Adding a broader token or making unsigned replacement commits would not
address the observed boundary.

## Implemented correction

- [PR #62](https://github.com/dlworrell/atarix/pull/62) established exact-SHA,
  valid-signature and single-parent verification, an `automation/` namespace
  restriction, refusal of `pull_request_target`, and `beforeOid` ref leases.
- [PR #76](https://github.com/dlworrell/atarix/pull/76) added credential redaction
  and a retained-commit recovery summary. Other API errors still fail.
- [PR #81](https://github.com/dlworrell/atarix/pull/81) and
  [AEMS PR #39](https://github.com/dlworrell/AEMS/pull/39) split publication:
  the report workflow needs only Contents write; the existing ring-0 App
  opens a PR after checking its configured branch, base, author, signature,
  single-commit shape and file allowlist. It does not approve or merge.
- This follow-up allows manual census recovery on `dev-daily`, matching the
  inventory workflow. PR events and other branches cannot publish census
  changes. CI now runs publisher regressions on every push and PR with
  Contents read, rather than waiting for the release workflow.

The report workflows receive no App private key, PAT, PR-write permission or
branch-protection bypass. Commit and leased-reference logic is unchanged.

## Before and after evidence

| Check | Before | Observed after the merged ring-0 correction |
|---|---|---|
| API rejection visibility | Generic exit status 1 | Exact policy rejection in run 40273574948; redacted errors and recovery summaries in #76 |
| Workflow PR access | PullRequests write, policy still refused creation | Report workflows request Contents write only |
| Inventory publication | Verified commit followed by failed PR creation | [Run 40292947378](https://github.com/dlworrell/atarix/actions/runs/40292947378) verified `a3718730ccd4c8ed44d7d93f50cc153f03fc5b61`; ring-0 opened [PR #83](https://github.com/dlworrell/atarix/pull/83) into dev-daily |
| Census unchanged output | A successful job could be mistaken for a new PR | [Run 40292947803](https://github.com/dlworrell/atarix/actions/runs/40292947803) explicitly reported no changes; it did not exercise new census PR creation |
| Manual census recovery | Publication condition accepted push only | Accepts push or manual dispatch on dev-daily |
| Publisher regressions | 15 tests, run automatically only at release | 19 tests, now included in normal CI |

The earlier report PRs [#63](https://github.com/dlworrell/atarix/pull/63) and
[#64](https://github.com/dlworrell/atarix/pull/64) are merged. They are distinct
from validation of the current ring-0 route. A green PR-event census run
checks generation and placement; it is not evidence of publisher execution.

## Reproduction and recovery

1. Run `python3 -m unittest discover -s tests -p 'test_publish_signed_pr.py' -v`.
   Coverage includes invalid signatures/parents, stale leases, policy and
   unrelated failures, redaction, no-op runs, existing PR reuse and ring-0
   handoff only after verification.
2. After merging this change into `dev-daily`, dispatch both report workflows
   on that branch. Confirm the publication step executed, the exact generated
   commit is verified with the expected parent, and the report paths alone
   changed. The new report record gives the census a real documentation change.
3. Inspect the next ring-0 sweep and the resulting PRs against `dev-daily`.
   A successful handoff is pending PR creation until the App actually opens it;
   a no-op census is successful but creates no PR.
4. If the sweep fails, retain the verified automation commit and inspect the
   AEMS sweep log. A maintainer can open its compare view without re-signing.
   Rollback uses a reviewed revert PR. Do not reset protected branches, grant
   broader credentials, or treat a queued handoff as a completed review.
