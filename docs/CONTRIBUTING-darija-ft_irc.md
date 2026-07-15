# GitHub Collaboration Guide for the ft_irc Team

Had guide kay3ref workflow simple dyal Git w GitHub l team sghira li katbni `ft_irc`. Ttsayeb bach tn9as merge conflicts, tkhalli `main` stable, w tkhalli kol contribution ttb9a traceable b branches w pull requests.[cite:5][cite:10]

## Repository setup

### 1. Create the repository

Sawb wa7ed shared GitHub repository li mamlook l wa7ed men teammates aw ta7t wa7ed GitHub organization. GitHub kaykhelli collaborators ytzado l public w private repositories f GitHub Free, w users li t3ardo 3lihom l invitation kayakhdo collaborator access mn ba3d ma kay9blo l invitation.[cite:5]

Recommended repository basics:

- Repository name: `ft_irc`
- Default branch: `main`
- Visibility: private ila l team بغات tkhalli l project internal
- Zid `README.md`, `.gitignore`, w license ghir ila l team ttaf9at; l school project, wa7ed minimal start غالبا kaykfi

### 2. Add collaborators

Repository owner khasso:

1. Y7ell repository f GitHub.
2. Ymchi l **Settings**.
3. Y7ell **Collaborators** f Access section.
4. Yklik 3la **Add people**.
5. Y9elleb 3la kol teammate b GitHub username aw email.
6. Ysift l invitation.
7. Kol teammate kay9bel l invite 9bel ma ypushi l code.[cite:5]

### 3. Protect the main branch

Dir wa7ed branch protection rule l `main`. GitHub branch protection t9dar tفرض pull request reviews, tفرض status checks, tفرض conversation resolution, tفرض linear history, w t9der تمنع force pushes aw deletion by default.[cite:10]

Recommended `main` protection settings:

- Require a pull request before merging.[cite:10]
- Require at least 1 approval before merging.[cite:10]
- Require conversation resolution before merging.[cite:10]
- Require linear history ila l team bghat history n9iya b squash aw rebase merges.[cite:10]
- Ma tkhallish **force pushes** l `main`.[cite:10]
- Ma tkhallish direct pushes l `main`; ga3 الناس kaykhdmo b pull requests.[cite:10]

## Branch strategy

Wa7ed lightweight feature-branch workflow kaynasb project dyal 3 d ناس 7sen men wa7ed heavy release model. Kol contributor kaykhdem 3la short-lived branches, kay7ell pull requests bzzaf, w kaymerge ghir service li tdaret lih review l `main`.[cite:10][cite:8]

### Branch types

St3mel had smiyat dyal branches b nafs tari9a:

| Branch type | Pattern | Purpose |
|---|---|---|
| Stable branch | `main` | Code li dayman kaycompile w mdwaz 3lih review |
| Feature branch | `feature/<name>` | New feature aw milestone work |
| Fix branch | `fix/<name>` | Bug fixes |
| Refactor branch | `refactor/<name>` | Internal cleanup bla ma tbeddel behavior |
| Test branch | `test/<name>` | Temporary test harnesses aw scripts |

Examples:

- `feature/server-bootstrap`
- `feature/poll-loop`
- `feature/client-registration`
- `fix/disconnect-cleanup`
- `refactor/logger-api`

### Ownership rules

Bach tn9as conflicts, kol member khasso ykoun 3ando wa7ed primary area dyalo w ytb3ed men editing dyal files dyal member akhor bla coordination. Hada kaytnaseb m3a team partitioning li deja t3erref l architecture, sockets, w utilities/parsing.[cite:5]

Suggested ownership:

- Ismail: server architecture, `Server.*`, `Client.*`, command dispatch, channels
- Ayoub: sockets, `poll()`, accept/disconnect flow, file descriptor management
- Souhail: Makefile, logger, utils, buffers, parser

Shared aw sensitive files khasshom ytbdlo b prior agreement:

- `main.cpp`
- `Server.hpp`
- shared enums / common interfaces
- build configuration

## Pull request workflow

Ga3 lkhadma khassha twsel l `main` men through pull requests. Branch protection t9dar tفرض reviews w t7بس merges 7tta ytkmlo requirements.[cite:10]

### Standard workflow

1. Bda men `main` li up-to-date.
2. Sna3 wa7ed dedicated branch l task wa7da.
3. Dir commits sghar w 3andhom ma3na.
4. Pushi l branch l GitHub.
5. 7ell pull request l `main`.
6. Tleb review men 9ell men teammate wa7ed.
7. Jaweb 3la comments.
8. Merge ghir ila t9bel w l branch mazal up to date.[cite:10]

### Pull request rules

Kol pull request khassha:

- Tghatti topic wa7ed wade7 fa9at.
- Tcompile بنجاح 9bel review.
- Tkon فيها wa7ed short description: ach tbeddel, achno files tbeddlo, kifach ttestat.
- Ttb9a sghira ila amkan bach review ykoun سريع.

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

Ila linear history mch3la f protected branch, GitHub kaytleb squash merge aw rebase merge f بلاصت merge commits.[cite:10]

Recommended choice for your team:

- St3mlo **Squash and merge** l aktar pull requests, 7it katkhalli `main` n9i w katjennb noisy history.[cite:10]

## Terminal workflow for each member

Had section hiya practical terminal handbook li kol member y9der ytb3ha.

### First-time setup

Clone repository:

```bash
git clone git@github.com:OWNER/ft_irc.git
cd ft_irc
```

Set identity ila mazal ma tconfiguratsh:

```bash
git config --global user.name "Your Name"
git config --global user.email "you@example.com"
```

Check remotes:

```bash
git remote -v
```

Explanation:

- `git clone`: kaydownloadi repository w history dyalha.
- `git config`: kaysetti author information li katban f commits.
- `git remote -v`: kaybayan fin kayشير `origin`.

### Daily start routine

Dayman bda b synchronizing local `main` m3a GitHub:

```bash
git checkout main
git pull origin main
```

Explanation:

- `git checkout main`: kaybdl l stable branch.
- `git pull origin main`: kayfetchi w kayدمج latest commits men GitHub l local `main`.

### Create a work branch

Sna3 branch men `main` li updated:

```bash
git checkout -b feature/client-registration
```

Explanation:

- `git checkout -b <branch>`: kaycreate branch jdida w kayswitchi liha f l7in.

### Work and inspect changes

Useful commands waqt development:

```bash
git status
git diff
git diff --staged
```

Explanation:

- `git status`: kaybayan modified, staged, w untracked files.
- `git diff`: kaybayan unstaged changes.
- `git diff --staged`: kaybayan ach ghadi ytkcommit.

### Stage and commit safely

Stage ghir files li ma9soudin:

```bash
git add src/Client.cpp include/Client.hpp
git commit -m "Add client registration state tracking"
```

Aw dir stage b interactive tari9a:

```bash
git add -p
```

Explanation:

- `git add <files>`: kaystage selected files.
- `git add -p`: kaystage changes chunk by chunk; hadchi mofid ila file wa7ed فيها بزاف dyal edits ma3ndhomsh 3la9a b ba3diyathom.
- `git commit -m`: kaysejjel wa7ed snapshot b message.

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

- `git push -u origin <branch>`: kayuploadi l branch w kaysetti upstream tracking.
- `git push`: kayصيفط commits jdod l tracked remote branch.

### Keep branch updated before PR

Fetch latest remote data:

```bash
git fetch origin
```

Dir rebase l branch dyalk فوق updated `main`:

```bash
git checkout feature/client-registration
git rebase origin/main
```

Ila kanu conflicts:

```bash
# edit files to resolve conflicts
git add <resolved-files>
git rebase --continue
```

Ila rebase wlla messy:

```bash
git rebase --abort
```

Explanation:

- `git fetch origin`: kaydownloadi remote updates bla merge.
- `git rebase origin/main`: kay3awed kayرتب current branch commits فوق latest `main`; hadchi kay3awn bach tb9a linear history.
- `git rebase --continue`: kaykammel mn ba3d ma tsla7 conflicts.
- `git rebase --abort`: kayrjja3 l branch l الحالة li kant 3liha 9bel.

Ila rebased branch kan deja tpusha, update l remote branch b aman:

```bash
git push --force-with-lease
```

Explanation:

- `git push --force-with-lease`: kayupdate remote branch mn ba3d rebase bla ma yoverwrite remote work b surprise. 3omrek ma تستعمل hadchi 3la `main`.[cite:10]

### Open and review pull requests

Mn ba3d ma tpushi, 7ell pull request f GitHub men branch dyalk l `main`. Branch protection t9dar tفرض approval w resolved conversations 9bel merge.[cite:10]

Reviewers khasshom ychekko:

- Wash kaycompile?
- Wash باقي f ownership area dyal author?
- Wash tbeddlet shared interface?
- Wash kaykسر assumptions dyal teammate akhor?
- Wash l branch mazal up to date m3a `main`?

## Conflict prevention rules

Had l rules aham men ay command:

- 3omrek ma tkhedem direct f `main`.[cite:10]
- 3omrek ma tpushi direct l `main`; st3mel ghir pull requests.[cite:10]
- Pulli `main` 9bel ma tsna3 branch jdida.
- Khalli branches short-lived.
- Ma tkhllatch khadma ma3ndhash 3la9a f nafs l branch.
- Nabbah teammates 9bel ma tbeddel shared headers aw interfaces.
- Faddel frequent small PRs 3la large late merges.
- 3omrek ma تستعمل `git push --force` 3la `main`.[cite:10]

## Conflict resolution from terminal

Mlli Git kayreporti conflict waqt merge aw rebase:

1. Dir `git status` bach tchof conflicted files.
2. 7ell kol file w 9elleb 3la conflict markers:

```txt
<<<<<<< HEAD
current branch code
=======
incoming code
>>>>>>> other-change
```

3. 9arrer final combined code m3a teammate ila kan l file shared.
4. 7yed markers.
5. Stage l resolved file.
6. Kammel l operation.

Commands:

```bash
git status
git add <resolved-files>
git rebase --continue
```

Aw, ila kan merge:

```bash
git commit
```

## Recovering from common mistakes

### Undo unstaged local changes

```bash
git restore path/to/file
```

Hadchi kayhiyed local unstaged edits f wa7ed l file.

### Unstage a file

```bash
git restore --staged path/to/file
```

Hadchi kayhiyed file men staging area bla ma kayms7 file changes.

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

Wa7ed simple written agreement kayjennb aktar Git drama:

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

Bach workflow ykoun explicit, zid had files l repo:

- `README.md`: project overview w quick start
- `CONTRIBUTING.md`: workflow rules men had guide
- `.gitignore`: build artifacts w local files
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

L wa7ed 3-person team li katkhdem 3la short timeline, asra3 stable workflow huwa:

1. Sna3 repository w zid ga3 l members.[cite:5]
2. Protecti `main` w tleb pull requests m3a review.[cite:10]
3. Zid had workflow guide b7al `CONTRIBUTING.md`.
4. Ttaf9o 3la file ownership w branch names f awal team call.
5. Bdiw b small PRs f ayam lawlin bach tvalidiw l process بكري.
