# Integration branch retention and recovery

Authority: [Catylist ADR-0013](https://github.com/dlworrell/Catylist/blob/main/docs/adr/ADR-0013-daily-fitment-and-release-branches.md),
extending ADR-0006 and using ADR-0007 shared pinned automation.
Status: diagnostic check; live integration-ruleset rollout still required.

## Incident and recovery

The central [AEMS Fitment Cycle failure](https://github.com/dlworrell/AEMS/actions/runs/40382778583/job/121783849779)
reported that `dev-daily` was missing in both AEMS and atarix. In atarix,
[promotion PR #86](https://github.com/dlworrell/atarix/pull/86) had merged the
daily head `822b990fed8f2aa070ca798e6fbfbe2de1866b32` into `fitment` at
`eaa04cd9a586bb21106042320f279f372ac79c2f`. The daily head is an ancestor of
that fitment merge, with one commit ahead and zero behind in the comparison.
After confirming the daily ref was absent, it was restored at that exact
recorded head, retaining the unreleased report-publication fixes and evidence.

Automatic deletion after merge was enabled, but `protect-integration-branches`
was absent and both integration branch names were unprotected. This supports
automatic deletion after promotion as the failure mechanism; no deletion audit
event was obtained to identify the deleting actor. Restoring from `main` would
have omitted the daily work. Existing branch histories were not rewritten.

## Read-only diagnostic

`.github/workflows/integration-branches.yml` uses the shared AEMS retention
action pinned to a full reviewed commit SHA. It reads both exact branch heads
and GitHub's effective active deletion rules, without checking out or executing
atarix source. It runs on PRs into the three governed branches, integration
branch pushes, manual dispatch and a daily drift check. Permissions remain
`contents: read`; no administration token or model key is copied here.

The check fails for a missing branch, an absent active deletion rule or
unavailable evidence. A generic `protected` flag does not prove retention.
It is a diagnostic, not a new mandatory check or complete ADR-0006 certification.
It does not create branches, edit settings, approve or merge a PR. The central
AEMS cycle uses the same preflight before repository mutations and promotions.

## Required settings rollout

Merging this PR installs the diagnostic; it does not apply repository rulesets.
The live check is expected to remain red until these settings are applied.

1. In AEMS **Apply Catylist Rulesets**, use repository filter
   `dlworrell/AEMS,dlworrell/atarix` and `dry_run: true`.
2. Pin the reviewed inventory and template sources. The inspected source
   commits for this repair were Catylist
   `863a50e8f85d462d3e9e064688a09a85fd2fcce2` and repo_templates
   `097338e20457b500fa66771091bbf6428457d10b`. Inspect any newer revision
   before substituting it.
3. Review the plan, run `dry_run: false`, and approve the existing
   `ruleset-admin` environment gate. Apply `protect-integration-branches`
   to `dev-daily` and `fitment`: no deletion, no force push, verified commits,
   PRs with merge commits only, and no bypass actors.
4. Dispatch **Integration branch retention** in atarix and a dry-run
   **Fitment Cycle** in AEMS. Require successful live evidence before the next
   owner-approved daily promotion; verify both long-lived refs survive it.
5. Keep the topic → `dev-daily` → `fitment` → `main` development path.
   The owner reviews and merges; agents never create the owner's approval.

Do not disable automatic deletion globally: ordinary merged topic branches
remain eligible for cleanup. If a long-lived ref disappears again, recover
its last verified head from promotion history after checking for newer lost
work. Never reset an existing branch automatically. Rollback is a reviewed
revert of this diagnostic; retain integration branch protection.

## Evidence and limits

The shared AEMS implementation passed all 92 focused retention/cycle/ruleset/
cleanup tests and the full 304-test AEMS suite. Controlled replay showed the
old cycle opening a promotion for unprotected existing branches while the
repair refused with zero repository writes; protected branches retained their
permitted behavior. The incident record and machine evidence live with AEMS.
An actual post-settings promotion remains an acceptance step, not a claimed
completed result. No hardware or product-readiness claim is made.

AI-assisted engineering work by OpenAI ChatGPT, following the Git Supervisor
and the Catylist-specific Git Workflow Master development plan. Changes are
submitted as signed topic commits into the recovered `dev-daily`; governance
approval and merging remain with the owner.
