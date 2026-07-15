# GitHub Collaboration Guide for the ft_irc Team

This guide defines a simple Git and GitHub workflow for a small team building `ft_irc`. It is designed to reduce merge conflicts, keep `main` stable, and make every contribution traceable through branches and pull requests.[cite:5][cite:10]

## Repository setup

### 1. Create the repository

Create one shared GitHub repository owned by one teammate or under a GitHub organization. GitHub allows collaborators to be invited to both public and private repositories on GitHub Free, and invited users gain collaborator access after accepting the invitation.[cite:5]

Recommended repository basics:

- Repository name: `ft_irc`
- Default branch: `main`
- Visibility: private if the team wants to keep the project internal
- Add a `README.md`, `.gitignore`, and license only if the team agrees; for a school project, a minimal start is usually enough

### 2. Add collaborators

The repository owner should:

1. Open the repository on GitHub.
2. Go to **Settings**.
3. Open **Collaborators** in the Access section.
4. Click **Add people**.
5. Search each teammate by GitHub username or email.
6. Send the invitation.
7. Each teammate accepts the invite before pushing code.[cite:5]

### 3. Protect the main branch

Set a branch protection rule for `main`. GitHub branch protection can require pull request reviews, require status checks, require conversation resolution, require linear history, and prevent force pushes or deletion by default.[cite:10]

Recommended `main` protection settings:

- Require a pull request before merging.[cite:10]
- Require at least 1 approval before merging.[cite:10]
- Require conversation resolution before merging.[cite:10]
- Require linear history if the team wants a clean history with squash or rebase merges.[cite:10]
- Do **not** allow force pushes to `main`.[cite:10]
- Do **not** allow direct pushes to `main`; everyone works through pull requests.[cite:10]

## Branch strategy

A lightweight feature-branch workflow fits a 3-person project better than a heavy release model. Each contributor works on short-lived branches, opens pull requests often, and merges only reviewed work into `main`.[cite:10][cite:8]

### Branch types

Use these branch names consistently:

| Branch type | Pattern | Purpose |
|---|---|---|
| Stable branch | `main` | Always compilable and review-approved code |
| Feature branch | `feature/<name>` | New feature or milestone work |
| Fix branch | `fix/<name>` | Bug fixes |
| Refactor branch | `refactor/<name>` | Internal cleanup without changing behavior |
| Test branch | `test/<name>` | Temporary test harnesses or scripts |

Examples:

- `feature/server-bootstrap`
- `feature/poll-loop`
- `feature/client-registration`
- `fix/disconnect-cleanup`
- `refactor/logger-api`

### Ownership rules

To reduce conflicts, every member should own a primary area and avoid editing another member's files without coordination. This matches the team partitioning already defined for architecture, sockets, and utilities/parsing.[cite:5]

Suggested ownership:

- Ismail: server architecture, `Server.*`, `Client.*`, command dispatch, channels
- Ayoub: sockets, `poll()`, accept/disconnect flow, file descriptor management
- Souhail: Makefile, logger, utils, buffers, parser

Shared or sensitive files should be modified with prior agreement:

- `main.cpp`
- `Server.hpp`
- shared enums / common interfaces
- build configuration

## Pull request workflow

All work should land in `main` through pull requests. Branch protection can enforce reviews and block merges until requirements are satisfied.[cite:10]

### Standard workflow

1. Start from an up-to-date `main`.
2. Create a dedicated branch for one task.
3. Commit in small, meaningful steps.
4. Push the branch to GitHub.
5. Open a pull request into `main`.
6. Request review from at least one teammate.
7. Address comments.
8. Merge only when approved and the branch is up to date.[cite:10]

### Pull request rules

Each pull request should:

- Cover one clear topic only.
- Compile successfully before review.
- Include a short description: what changed, what files changed, how it was tested.
- Be kept small when possible so review is fast.

Suggested PR template:

```md
## What changed
- Added ...
- Refactored ...

## Why
- Needed for Phase X / integration with Y

## Files touched
- Server.cpp
- SocketManager.cpp

## How it was tested
- make re
- manual connection with nc
- tested disconnect flow

## Notes for reviewers
- Review the interface in Server.hpp first
```

### Merge method

If linear history is enabled on the protected branch, GitHub requires squash merge or rebase merge instead of merge commits.[cite:10]

Recommended choice for your team:

- Use **Squash and merge** for most pull requests, because it keeps `main` clean and avoids a noisy history.[cite:10]

## Terminal workflow for each member

This section is the practical terminal handbook each member can follow.

### First-time setup

Clone the repository:

```bash
git clone git@github.com:OWNER/ft_irc.git
cd ft_irc
```

Set identity if not already configured:

```bash
git config --global user.name "Your Name"
git config --global user.email "you@example.com"
```

Check remotes:

```bash
git remote -v
```

Explanation:

- `git clone`: downloads the repository and its history.
- `git config`: sets the author information that appears in commits.
- `git remote -v`: shows where `origin` points.

### Daily start routine

Always start by synchronizing local `main` with GitHub:

```bash
git checkout main
git pull origin main
```

Explanation:

- `git checkout main`: switches to the stable branch.
- `git pull origin main`: fetches and integrates the latest commits from GitHub into local `main`.

### Create a work branch

Create a branch from updated `main`:

```bash
git checkout -b feature/client-registration
```

Explanation:

- `git checkout -b <branch>`: creates a new branch and switches to it immediately.

### Work and inspect changes

Useful commands during development:

```bash
git status
git diff
git diff --staged
```

Explanation:

- `git status`: shows modified, staged, and untracked files.
- `git diff`: shows unstaged changes.
- `git diff --staged`: shows what will be committed.

### Stage and commit safely

Stage only intended files:

```bash
git add src/Client.cpp include/Client.hpp
git commit -m "Add client registration state tracking"
```

Or stage interactively:

```bash
git add -p
```

Explanation:

- `git add <files>`: stages selected files.
- `git add -p`: stages changes chunk by chunk; this is useful when one file contains multiple unrelated edits.
- `git commit -m`: records a snapshot with a message.

Suggested commit style:

- `Add poll loop for client events`
- `Implement CRLF extraction in BufferManager`
- `Fix client cleanup on disconnect`
- `Refactor logger interface`

### Push branch to GitHub

First push:

```bash
git push -u origin feature/client-registration
```

Next pushes:

```bash
git push
```

Explanation:

- `git push -u origin <branch>`: uploads the branch and sets upstream tracking.
- `git push`: sends new commits to the tracked remote branch.

### Keep branch updated before PR

Fetch the latest remote data:

```bash
git fetch origin
```

Rebase your branch on top of updated `main`:

```bash
git checkout feature/client-registration
git rebase origin/main
```

If there are conflicts:

```bash
# edit files to resolve conflicts
git add <resolved-files>
git rebase --continue
```

If the rebase becomes messy:

```bash
git rebase --abort
```

Explanation:

- `git fetch origin`: downloads remote updates without merging them.
- `git rebase origin/main`: replays the current branch commits on top of the latest `main`; this helps keep a linear history.
- `git rebase --continue`: continues after conflicts are fixed.
- `git rebase --abort`: restores the branch to its previous state.

If a rebased branch was already pushed, update the remote branch safely:

```bash
git push --force-with-lease
```

Explanation:

- `git push --force-with-lease`: updates the remote branch after rebase while refusing to overwrite remote work unexpectedly. Never use this on `main`.[cite:10]

### Open and review pull requests

After pushing, open a pull request on GitHub from your branch into `main`. Branch protection can require approval and resolved conversations before merge.[cite:10]

Reviewers should check:

- Does it compile?
- Does it stay inside the author's ownership area?
- Did a shared interface change?
- Does it break another teammate's assumptions?
- Is the branch up to date with `main`?

## Conflict prevention rules

These rules matter more than any command:

- Never code directly on `main`.[cite:10]
- Never push directly to `main`; use pull requests only.[cite:10]
- Pull `main` before creating a new branch.
- Keep branches short-lived.
- Do not mix unrelated work in one branch.
- Warn teammates before changing shared headers or interfaces.
- Prefer frequent small PRs over large late merges.
- Never use `git push --force` on `main`.[cite:10]

## Conflict resolution from terminal

When Git reports a conflict during merge or rebase:

1. Run `git status` to see conflicted files.
2. Open each file and find conflict markers:

```txt
<<<<<<< HEAD
current branch code
=======
incoming code
>>>>>>> other-change
```

3. Decide the final combined code with the teammate if the file is shared.
4. Remove the markers.
5. Stage the resolved file.
6. Continue the operation.

Commands:

```bash
git status
git add <resolved-files>
git rebase --continue
```

Or, for a merge:

```bash
git commit
```

## Recovering from common mistakes

### Undo unstaged local changes

```bash
git restore path/to/file
```

This discards local unstaged edits in a file.

### Unstage a file

```bash
git restore --staged path/to/file
```

This removes a file from the staging area without deleting the file changes.

### See branch list

```bash
git branch
git branch -r
git branch -a
```

- `git branch`: local branches.
- `git branch -r`: remote branches.
- `git branch -a`: all branches.

### Delete a local branch after merge

```bash
git checkout main
git pull origin main
git branch -d feature/client-registration
```

### Delete the remote branch after merge

```bash
git push origin --delete feature/client-registration
```

## Suggested team agreement

A simple written agreement avoids most Git drama:

| Rule | Team agreement |
|---|---|
| Stable branch | `main` only |
| Direct pushes to `main` | Forbidden |
| Merge path | Pull request only |
| Required review | 1 teammate minimum |
| Shared header changes | Announce before pushing |
| Branch naming | `feature/`, `fix/`, `refactor/`, `test/` |
| Preferred merge | Squash merge |
| Force push | Allowed only on personal feature branches, never on `main` |
| PR size | Small and focused |
| Before every session | `git checkout main && git pull origin main` |

## Recommended repository files

To make the workflow explicit, add these files to the repo:

- `README.md`: project overview and quick start
- `CONTRIBUTING.md`: workflow rules from this guide
- `.gitignore`: build artifacts and local files
- optionally `.github/pull_request_template.md`: standard PR checklist

## Minimal command cheat sheet

```bash
# first time
git clone git@github.com:OWNER/ft_irc.git
cd ft_irc

# start a session
git checkout main
git pull origin main
git checkout -b feature/my-task

# work
git status
git add <files>
git commit -m "Describe the change"
git push -u origin feature/my-task

# sync before merge
git fetch origin
git rebase origin/main
git push --force-with-lease
```

## Recommended adoption plan

For a 3-person team working on a short timeline, the fastest stable workflow is:

1. Create the repository and invite all members.[cite:5]
2. Protect `main` and require pull requests with review.[cite:10]
3. Add this workflow guide as `CONTRIBUTING.md`.
4. Agree on file ownership and branch names in the first team call.
5. Start with small PRs during the first days to validate the process early.

