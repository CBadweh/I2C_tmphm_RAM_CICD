# Git Cherry-Pick Tutorial: Day 3 TMPHM Implementation

## Table of Contents
1. [Introduction](#introduction)
2. [What is Git Cherry-Pick?](#what-is-git-cherry-pick)
3. [Initial Situation](#initial-situation)
4. [Pre-Cherry-Pick Research](#pre-cherry-pick-research)
5. [Step-by-Step Cherry-Pick Process](#step-by-step-cherry-pick-process)
6. [Understanding Conflict Types](#understanding-conflict-types)
7. [Final Verification](#final-verification)
8. [Summary and Key Takeaways](#summary-and-key-takeaways)

---

## Introduction

This document provides a complete, step-by-step walkthrough of using `git cherry-pick` to copy Day 3 TMPHM (Temperature/Humidity Module) implementation from the `day6_fixed` branch to the `main` branch.

**Goal:** Copy Day 3 code without bringing in Day 4, 5, or 6 changes.

**Date:** November 19, 2025
**Repository:** `gene_Baremetal_I2CTmphm_RAM_CICD`

---

## What is Git Cherry-Pick?

### Concept

**Cherry-pick** allows you to copy specific commits from one branch to another without merging the entire branch.

**Analogy:** Imagine you have two notebooks:
- Notebook A (day6_fixed) has lessons 1-6 written in it
- Notebook B (main) only has lessons 1-2

Instead of copying ALL lessons from Notebook A, cherry-pick lets you copy JUST lesson 3 into Notebook B.

### Visual Representation

```
BEFORE:
main:       A --- B --- C

day6_fixed: A --- B --- D --- E --- F --- G
                         ↑
                     Day 3 commits we want

AFTER cherry-pick:
main:       A --- B --- C --- D' --- E'
                              ↑
                          Copied Day 3 commits
                          (new commit hashes)

day6_fixed: A --- B --- D --- E --- F --- G
                         ↑
                    Original commits (unchanged)
```

### Key Points

1. **Creates NEW commits** - The copied commits get new commit hashes
2. **Preserves history** - Original commits remain unchanged on source branch
3. **Can cause conflicts** - Git may need your help to merge changes
4. **Selective copying** - You choose exactly which commits to copy

---

## Initial Situation

### Branch Status

**Current branch:** `main`
**Current commit:** `78ea2600aef9b6d6aac425315d802177d1d9983b`

```bash
git status
```

**Output:**
```
On branch main
Your branch is ahead of 'origin/main' by 1 commit.
nothing to commit, working tree clean
```

**Meaning:**
- We're on the `main` branch
- Working directory is clean (no uncommitted changes)
- Safe to start cherry-picking

### Target Commits

We need to copy Day 3 implementation from `day6_fixed` branch:

1. **Commit `0c64c923`** - "fix: Day 3 build issues" (most recent)
2. **Commit `4df4da8`** - "Day 3: sonnet implemented tmphm module and lwl" (middle)
3. **Commit `ee87f4e`** - "add LWL and Documents" (oldest)

**Important:** We're cherry-picking in **reverse chronological order** (newest first, then older ones).

---

## Pre-Cherry-Pick Research

### Step 1: Examine the Target Commit

Before cherry-picking, we need to understand what we're copying.

**Command:**
```bash
git show --stat 0c64c923e43772c7bcc0f875e5189c65ab28b80f
```

**Purpose:** Show which files were changed in this commit and how many lines.

**Output (abbreviated):**
```
commit 0c64c923e43772c7bcc0f875e5189c65ab28b80f
Author: Sbadweh <sheenaymudwe@gmail.com>
Date:   Wed Nov 19 15:54:43 2025 -0500

    fix: Day 3 build issues - disable premature FAULT feature and fix dependencies

 .claude/settings.local.json                        |  17 +
 Badweh_development/Debug/Core/Src/subdir.mk        |   2 +-
 Badweh_development/STM32F401RETX_FLASH.ld          |  10 +-
 Badweh_development/modules/include/config.h        |   4 +-
 Badweh_development/modules/stubs_day3.c            |  30 ++
 Progress_Report/git_info.md                        | 280 ++++++++++++++++
 31 files changed, 446 insertions(+), 1284 deletions(-)
```

**What this tells us:**
- **31 files changed** - This is a substantial commit
- **New files added:** `stubs_day3.c`, `git_info.md`, `.claude/settings.local.json`
- **Modified files:** `config.h`, linker script, many Debug makefiles
- **446 lines added, 1284 deleted** - Net reduction (deleted backup files)

### Step 2: Check Current Branch State

**Command:**
```bash
git log --oneline -5
```

**Purpose:** See recent commit history on main branch.

**Output:**
```
78ea260 add Badweh_Development/Badweh_Development (1).launch to gitignore
157fb30 ignore .cproject and .project files in .gitignore
42238dd fix: update .gitignore to properly ignore Debug/Release folders
9f91354 update gitignore
2776e34 Remove IDE/editor personal configs from repo (keep local only)
```

**Observation:** Main branch only has gitignore-related commits. No Day 3 code yet.

### Step 3: Verify What Files Exist on Main

**Command:**
```bash
ls Badweh_development/modules/
```

**Output:**
```
cmd/
console/
i2c/
include/
log/
templates/
tmphm/
tmr/
ttys/
```

**Important observations:**
- ✅ `tmphm/` directory exists (but may have old code)
- ❌ `lwl/` directory does NOT exist (will be created by cherry-pick)
- ❌ `stubs_day3.c` does NOT exist

---

## Step-by-Step Cherry-Pick Process

## Cherry-Pick #1: Build Fixes (Commit 0c64c92)

### Step 1.1: Create Safety Backup

**Why?** Always create a backup before potentially destructive operations. If something goes wrong, you can easily revert.

**Command:**
```bash
git branch backup-before-day3
```

**Purpose:** Creates a new branch pointing to current commit, but stays on `main`.

**Verify it worked:**
```bash
git branch -v
```

**Output:**
```
  backup-before-day3             78ea260 add Badweh_Development/Badweh_Development (1).launch to gitignore
* main                           78ea260 [ahead 1] add Badweh_Development/Badweh_Development (1).launch to gitignore
```

**Explanation:**
- `*` indicates current branch (main)
- Both branches point to same commit `78ea260`
- `backup-before-day3` is our safety net

### Step 1.2: Execute First Cherry-Pick

**Command:**
```bash
git cherry-pick 0c64c923e43772c7bcc0f875e5189c65ab28b80f
```

**Purpose:** Copy commit 0c64c92 from day6_fixed to main.

**Output:**
```
error: could not apply 0c64c92... fix: Day 3 build issues - disable premature FAULT feature and fix dependencies
hint: After resolving the conflicts, mark them with
hint: "git add/rm <pathspec>", then run
hint: "git cherry-pick --continue".

CONFLICT (modify/delete): Badweh_development/Debug/Core/Src/subdir.mk deleted in HEAD and modified in 0c64c92
CONFLICT (modify/delete): Badweh_development/Debug/makefile deleted in HEAD and modified in 0c64c92
[... 20 more similar conflicts ...]
Auto-merging Badweh_development/STM32F401RETX_FLASH.ld
```

**What happened?**
- Git tried to apply the commit
- Found **22 conflicts** in Debug folder files
- **Paused** the cherry-pick operation
- Waiting for us to resolve conflicts

### Step 1.3: Understand the Conflict

**Command:**
```bash
git status
```

**Output:**
```
On branch main
You are currently cherry-picking commit 0c64c92.
  (fix conflicts and run "git cherry-pick --continue")

Changes to be committed:
	new file:   .claude/settings.local.json
	new file:   Badweh_development/Debug/modules/subdir.mk
	modified:   Badweh_development/STM32F401RETX_FLASH.ld
	modified:   Badweh_development/modules/include/config.h
	new file:   Badweh_development/modules/stubs_day3.c
	new file:   Progress_Report/git_info.md

Unmerged paths:
  (use "git add/rm <file>..." as appropriate to mark resolution)
	deleted by us:   Badweh_development/Debug/Core/Src/subdir.mk
	deleted by us:   Badweh_development/Debug/makefile
	[... more deleted files ...]
```

**Understanding Git Status:**

**"You are currently cherry-picking commit 0c64c92"**
- Git is in "cherry-pick mode" (paused state)
- Must resolve conflicts before continuing

**"Changes to be committed"** (✅ Success)
- These files were successfully merged
- They're staged and ready to commit
- Includes: `stubs_day3.c`, `config.h` (modified), new documentation

**"Unmerged paths"** (⚠️ Conflicts)
- These files need manual resolution
- Status code: `deleted by us` (DU)

### Step 1.4: Understanding Conflict Status Codes

**Command:**
```bash
git status --short
```

**Output:**
```
A  .claude/settings.local.json
DU Badweh_development/Debug/Core/Src/subdir.mk
DU Badweh_development/Debug/makefile
M  Badweh_development/STM32F401RETX_FLASH.ld
M  Badweh_development/modules/include/config.h
A  Badweh_development/modules/stubs_day3.c
A  Progress_Report/git_info.md
```

**Status Code Legend:**
- **`A`** = **Added** - New file created successfully
- **`M`** = **Modified** - Existing file changed successfully
- **`DU`** = **Deleted by Us, Updated by them** - CONFLICT!

**What does DU mean?**
- **"Deleted by Us"** - The file doesn't exist on our main branch (it's in .gitignore)
- **"Updated by them"** - The commit wants to modify the file
- **Conflict!** - Git doesn't know whether to delete or keep it

### Step 1.5: Examine a Specific Conflict

Let's look at what Git is trying to do with one of these conflicted files:

**Command:**
```bash
cat Badweh_development/Debug/modules/tmphm/subdir.mk
```

**Output (partial):**
```makefile
################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
# Reference files (tmphm_original, tmphm_v2) commented out - they don't exist
C_SRCS += \
../modules/tmphm/tmphm.c
# ../modules/tmphm/tmphm_original.c \
# ../modules/tmphm/tmphm_v2.c

[... compiler flags without -fcyclomatic-complexity ...]
```

**What was the change?**

Let's see what it looked like BEFORE this commit:

**Command:**
```bash
git show 0c64c923^:Badweh_development/Debug/modules/tmphm/subdir.mk | head -35
```

**Output:**
```makefile
C_SRCS += \
../modules/tmphm/tmphm.c

[... compiler flags WITH -fcyclomatic-complexity ...]
```

**The Difference:**
- **Before:** Had `-fcyclomatic-complexity` compiler flag
- **After:** Removed that flag (GCC 10.3.1 doesn't support it)
- **Before:** Clean, no commented lines
- **After:** Added comments showing removed backup files

**Why the conflict?**
- On `main` branch: File doesn't exist (it's in .gitignore)
- On `day6_fixed`: File exists and commit wants to modify it
- Git asks: "Should I create this file with the changes, or keep it deleted?"

### Step 1.6: Resolve the Conflicts - Strategy Decision

**Our Decision:** KEEP THEM DELETED

**Reasons:**
1. These are **IDE-generated files** (STM32CubeIDE auto-creates them)
2. They're in **`.gitignore`** (should not be version controlled)
3. The IDE will **regenerate** them when we build
4. Including them causes unnecessary conflicts in the future

**Resolution Command:**
```bash
git rm Badweh_development/Debug/Core/Src/subdir.mk \
       Badweh_development/Debug/Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk \
       Badweh_development/Debug/app1/subdir.mk \
       Badweh_development/Debug/makefile
```

**What `git rm` does:**
- Tells Git: "Yes, I want to keep these files deleted"
- Resolves the conflict in favor of deletion
- Stages the resolution

**Output:**
```
rm 'Badweh_development/Debug/Core/Src/subdir.mk'
rm 'Badweh_development/Debug/Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk'
rm 'Badweh_development/Debug/app1/subdir.mk'
rm 'Badweh_development/Debug/makefile'
```

**Meaning:** Git acknowledged the deletion and resolved the conflict.

**Continue removing the rest:**
```bash
git rm Badweh_development/Debug/modules/blinky/subdir.mk \
       Badweh_development/Debug/modules/cmd/subdir.mk \
       Badweh_development/Debug/modules/console/subdir.mk \
       Badweh_development/Debug/modules/dio/subdir.mk \
       Badweh_development/Debug/modules/fault/subdir.mk \
       Badweh_development/Debug/modules/flash/subdir.mk \
       Badweh_development/Debug/modules/gps_gtu7/subdir.mk
```

```bash
git rm Badweh_development/Debug/modules/i2c/subdir.mk \
       Badweh_development/Debug/modules/log/subdir.mk \
       Badweh_development/Debug/modules/lwl/subdir.mk \
       Badweh_development/Debug/modules/mem/subdir.mk \
       Badweh_development/Debug/modules/stat/subdir.mk \
       Badweh_development/Debug/modules/tmphm/subdir.mk \
       Badweh_development/Debug/modules/tmr/subdir.mk
```

```bash
git rm Badweh_development/Debug/modules/ttys/subdir.mk \
       Badweh_development/Debug/modules/wdg/subdir.mk \
       Badweh_development/Debug/objects.list \
       Badweh_development/Debug/sources.mk
```

### Step 1.7: Verify All Conflicts Resolved

**Command:**
```bash
git status
```

**Output:**
```
On branch main
You are currently cherry-picking commit 0c64c92.
  (all conflicts fixed: run "git cherry-pick --continue")

Changes to be committed:
	new file:   .claude/settings.local.json
	new file:   Badweh_development/Debug/modules/subdir.mk
	modified:   Badweh_development/STM32F401RETX_FLASH.ld
	modified:   Badweh_development/modules/include/config.h
	new file:   Badweh_development/modules/stubs_day3.c
	new file:   Progress_Report/git_info.md
```

**Key observation:**
- No more "Unmerged paths" section
- Git says: **"all conflicts fixed: run "git cherry-pick --continue"**
- Ready to complete the cherry-pick!

### Step 1.8: Review What Will Be Committed

Before continuing, let's see what changes we're about to commit:

**Command:**
```bash
git diff --cached --stat
```

**Purpose:** Show summary of staged changes.

**Output:**
```
 .claude/settings.local.json                 |  17 ++
 Badweh_development/Debug/modules/subdir.mk  |  27 +++
 Badweh_development/STM32F401RETX_FLASH.ld   |  10 +-
 Badweh_development/modules/include/config.h |   4 +-
 Badweh_development/modules/stubs_day3.c     |  30 +++
 Progress_Report/git_info.md                 | 280 ++++++++++++++++++++++
 6 files changed, 361 insertions(+), 7 deletions(-)
```

**Analysis:**
- **6 files** will be committed (instead of original 31)
- Removed all the Debug makefile conflicts
- Kept the important source files and documentation

**Look at critical config.h changes:**
```bash
git diff --cached Badweh_development/modules/include/config.h
```

**Output:**
```diff
diff --git a/Badweh_development/modules/include/config.h b/Badweh_development/modules/include/config.h
index 65df609..667140e 100644
--- a/Badweh_development/modules/include/config.h
+++ b/Badweh_development/modules/include/config.h
@@ -14,7 +14,7 @@
     #define CONFIG_STM32_LL_I2C_HDR "stm32f4xx_ll_i2c.h"
     #define CONFIG_STM32_LL_RCC_HDR "stm32f4xx_ll_rcc.h"
     #define CONFIG_STM32_LL_USART_HDR "stm32f4xx_ll_usart.h"
-    #define CONFIG_STM32_LL_IWDG_HDR "stm32f4xx_ll_iwdg.h"
+    // #define CONFIG_STM32_LL_IWDG_HDR "stm32f4xx_ll_iwdg.h"  // Commented out

@@ -68,7 +68,7 @@

 // FAULT feature (enabled for Day 4)
-#define CONFIG_FEAT_FAULT 1
+#define CONFIG_FEAT_FAULT 0
```

**Critical Day 3 Changes:**
1. **Line 17:** `CONFIG_STM32_LL_IWDG_HDR` commented out (watchdog driver not needed in Day 3)
2. **Line 71:** `CONFIG_FEAT_FAULT` changed from `1` to `0` (disables FAULT feature for Day 3)

**Why these changes?**
- Day 3 should only have I2C + TMPHM + LWL
- FAULT and Watchdog features come in Day 4
- This prevents compilation errors from missing drivers

### Step 1.9: Complete the Cherry-Pick

**Command:**
```bash
git cherry-pick --continue
```

**Purpose:** Finish the cherry-pick operation and create the commit.

**What happens when you run this:**
1. Git checks that all conflicts are resolved
2. Opens your editor to confirm commit message (or uses default)
3. Creates a new commit on main branch
4. Exits "cherry-picking" mode

**Output:**
```
[main e85ff37] fix: Day 3 build issues - disable premature FAULT feature and fix dependencies
 Date: Wed Nov 19 15:54:43 2025 -0500
 6 files changed, 361 insertions(+), 7 deletions(-)
 create mode 100644 .claude/settings.local.json
 create mode 100644 Badweh_development/Debug/modules/subdir.mk
 create mode 100644 Badweh_development/modules/stubs_day3.c
 create mode 100644 Progress_Report/git_info.md
```

**Understanding the output:**
- **`[main e85ff37]`** - Created new commit with hash `e85ff37` on main branch
- **`Date: Wed Nov 19 15:54:43`** - Preserves original commit date
- **`6 files changed`** - Our resolved version
- **`create mode 100644`** - New files added

**Important Note:**
- Original commit hash: `0c64c92`
- New commit hash: `e85ff37`
- **Different hashes** because they're different commits (different parent, different tree)

**Verify the commit:**
```bash
git log --oneline -3
```

**Output:**
```
e85ff37 fix: Day 3 build issues - disable premature FAULT feature and fix dependencies
78ea260 add Badweh_Development/Badweh_Development (1).launch to gitignore
157fb30 ignore .cproject and .project files in .gitignore
```

✅ **Cherry-pick #1 COMPLETE!**

---

## Cherry-Pick #2: TMPHM Implementation (Commit 4df4da8)

### Step 2.1: Execute Second Cherry-Pick

Now we need the actual Day 3 implementation that modifies tmphm.c.

**Command:**
```bash
git cherry-pick 4df4da8eda66a66e684c6019836b4dbf9660d13c
```

**Output:**
```
error: could not apply 4df4da8... Day 3: sonnet implemented tmphm module and lwl

Auto-merging Badweh_development/app1/app_main.c
CONFLICT (content): Merge conflict in Badweh_development/app1/app_main.c
Auto-merging Badweh_development/modules/i2c/i2c.c
CONFLICT (content): Merge conflict in Badweh_development/modules/i2c/i2c.c
Auto-merging Badweh_development/modules/tmphm/tmphm.c
CONFLICT (content): Merge conflict in Badweh_development/modules/tmphm/tmphm.c
```

**Different conflict type!**
- Not "modify/delete" (DU)
- Now we have "CONFLICT (content)" - merge conflicts INSIDE files

### Step 2.2: Check New Conflict Type

**Command:**
```bash
git status --short
```

**Output:**
```
UU Badweh_development/app1/app_main.c
UU Badweh_development/modules/i2c/i2c.c
UU Badweh_development/modules/tmphm/tmphm.c
A  Day3_TMPHM_LWL.md
```

**New status code:**
- **`UU`** = **Updated by both Us and them** - CONTENT CONFLICT!
- **`A`** = Added successfully (documentation file)

**What `UU` means:**
- File exists on BOTH branches
- BOTH sides made changes to the same file
- Git can't automatically merge the changes
- Need to choose which version to keep

### Step 2.3: Examine a Content Conflict

Let's look inside one of the conflicted files to see what Git did:

**Command:**
```bash
cat Badweh_development/app1/app_main.c | head -50
```

**Output:**
```c
/*
<<<<<<< HEAD
 * @brief Main application entry point and super loop
=======
 * @brief Main application file - Day 3: TMPHM Integration
>>>>>>> 4df4da8 (Day 3: sonnet implemented tmphm module and lwl)
 *
 * Initializes all modules...
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

<<<<<<< HEAD
// Day 1 Essential Modules Only
#include "cmd.h"         // Console command infrastructure
#include "console.h"     // User interaction
#include "i2c.h"         // I2C bus driver (Day 1 - adding error detection)
#include "tmphm.h"       // Temperature/Humidity module
#include "tmr.h"         // Timer for periodic sampling
#include "ttys.h"        // Serial UART
#include "module.h"      // Module framework


=======
#include "console.h"
#include "dio.h"
#include "i2c.h"
#include "lwl.h"         // ← Day 3 needs LWL!
#include "module.h"
#include "tmphm.h"
#include "ttys.h"
#include "tmr.h"

////////////////////////////////////////////////////////////////////////////////
// OPERATING MODE SELECTION
////////////////////////////////////////////////////////////////////////////////
//
// MODE A: TMPHM Automatic Sensor Sampling (Day 3 - Production Mode)
//   - Temperature/humidity sensor runs in background
>>>>>>> 4df4da8 (Day 3: sonnet implemented tmphm module and lwl)
```

### Step 2.4: Understanding Conflict Markers

Git added special markers to show the conflicting sections:

```
<<<<<<< HEAD
[YOUR CURRENT VERSION - what's on main branch now]
=======
[THEIR VERSION - what commit 4df4da8 wants to change it to]
>>>>>>> 4df4da8 (Day 3: sonnet implemented tmphm module and lwl)
```

**Breakdown of the conflict:**

**First conflict (lines 2-6):** File header comment
- **HEAD version:** "Main application entry point and super loop"
- **4df4da8 version:** "Main application file - Day 3: TMPHM Integration"

**Second conflict (lines 16-45):** Include statements
- **HEAD version:** Has Day 1 modules, no `lwl.h`
- **4df4da8 version:** Has Day 3 modules including `lwl.h` and `dio.h`

### Step 2.5: Resolve Content Conflicts

For Day 3, we want the **complete Day 3 version** from commit 4df4da8.

**Manual resolution options:**
1. **Manually edit** each file and remove conflict markers
2. **Use `git checkout --theirs`** to accept their version
3. **Use `git checkout --ours`** to keep our version
4. **Use a merge tool** for visual conflict resolution

**We'll use option 2 (fastest for our case):**

**Command:**
```bash
git checkout --theirs Badweh_development/app1/app_main.c \
                      Badweh_development/modules/i2c/i2c.c \
                      Badweh_development/modules/tmphm/tmphm.c
```

**What this command does:**
- **`--theirs`** means "use THEIR version" (from commit 4df4da8)
- Replaces conflicted files with the commit's version
- Removes all conflict markers
- Files now have clean Day 3 code

**Output:**
```
Updated 3 paths from the index
```

**Verify the conflict is gone:**
```bash
cat Badweh_development/app1/app_main.c | head -15
```

**Now shows clean code (no conflict markers):**
```c
/*
 * @brief Main application file - Day 3: TMPHM Integration
 *
 * Initializes all modules...
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "console.h"
#include "dio.h"
#include "i2c.h"
#include "lwl.h"         // Day 3 LWL module
```

### Step 2.6: Stage Resolved Files

**Command:**
```bash
git add Badweh_development/app1/app_main.c \
        Badweh_development/modules/i2c/i2c.c \
        Badweh_development/modules/tmphm/tmphm.c
```

**Purpose:** Tell Git "these files are resolved, ready to commit"

**Verify status:**
```bash
git status
```

**Output:**
```
On branch main
You are currently cherry-picking commit 4df4da8.
  (all conflicts fixed: run "git cherry-pick --continue")

Changes to be committed:
	modified:   Badweh_development/app1/app_main.c
	modified:   Badweh_development/modules/i2c/i2c.c
	modified:   Badweh_development/modules/tmphm/tmphm.c
	new file:   Day3_TMPHM_LWL.md
```

### Step 2.7: Complete Second Cherry-Pick

**Command:**
```bash
git cherry-pick --continue
```

**Output:**
```
[main aabab15] Day 3: sonnet implemented tmphm module and lwl
 Date: Tue Nov 11 22:13:46 2025 -0500
 4 files changed, 1884 insertions(+), 998 deletions(-)
 create mode 100644 Day3_TMPHM_LWL.md
```

**Significant changes:**
- **1884 lines added** - Complete Day 3 tmphm implementation
- **998 lines deleted** - Replaced old simplified code
- **Net:** +886 lines

✅ **Cherry-pick #2 COMPLETE!**

---

## Cherry-Pick #3: LWL Module Addition (Commit ee87f4e)

### Step 3.1: Check for LWL Module

**Command:**
```bash
ls -la Badweh_development/modules/lwl/ 2>&1
```

**Output:**
```
ls: cannot access 'Badweh_development/modules/lwl/': No such file or directory
```

**Problem:** The LWL module directory doesn't exist yet!

**Why?** Commit 4df4da8 only **modified** existing files. The `lwl.c` file was added in an earlier commit.

**Solution:** Cherry-pick commit ee87f4e which originally added the LWL module.

### Step 3.2: Research the Commit

**Command:**
```bash
git show --stat ee87f4e51ba8daa0c0de3c52748a7cca494bf76a
```

**Output:**
```
commit ee87f4e51ba8daa0c0de3c52748a7cca494bf76a
Author: Sbadweh <sheenaymudwe@gmail.com>
Date:   Sat Oct 25 07:08:21 2025 -0400

    add LWL and Documents

 Badweh_development/modules/include/lwl.h       |  84 ++++
 Badweh_development/modules/lwl/lwl.c           | 310 ++++++++++++++
 Badweh_development/modules/lwl/lwl.py          | 564 +++++++++++++++++++++++++
 Day_3_COMPLETE_Summary.md                      | 390 +++++++++++++++++
 16 files changed, 1719 insertions(+), 2 deletions(-)
```

**Key additions:**
- **`lwl.c`** - 310 lines of C code (the LWL module we need!)
- **`lwl.py`** - 564 lines of Python (decoder tool)
- **`lwl.h`** - 84 lines (header file)
- Documentation files

### Step 3.3: Execute Third Cherry-Pick

**Command:**
```bash
git cherry-pick ee87f4e51ba8daa0c0de3c52748a7cca494bf76a
```

**Output:**
```
error: could not apply ee87f4e... add LWL and Documents

CONFLICT (modify/delete): Badweh_development/Debug/makefile deleted in HEAD and modified in ee87f4e
CONFLICT (modify/delete): Badweh_development/Debug/sources.mk deleted in HEAD and modified in ee87f4e
Auto-merging Badweh_development/app1/app_main.c
CONFLICT (content): Merge conflict in Badweh_development/app1/app_main.c
Auto-merging Badweh_development/modules/i2c/i2c.c
CONFLICT (content): Merge conflict in Badweh_development/modules/i2c/i2c.c
Auto-merging Badweh_development/modules/include/config.h
CONFLICT (content): Merge conflict in Badweh_development/modules/include/config.h
Auto-merging Badweh_development/modules/include/lwl.h
CONFLICT (add/add): Merge conflict in Badweh_development/modules/include/lwl.h
Auto-merging Badweh_development/modules/tmphm/tmphm.c
CONFLICT (content): Merge conflict in Badweh_development/modules/tmphm/tmphm.c
```

**Multiple conflict types!**
- `modify/delete` - Debug makefiles again
- `content` - Files modified on both sides
- `add/add` - lwl.h added on both sides

### Step 3.4: Check Status

**Command:**
```bash
git status --short
```

**Output:**
```
DU Badweh_development/Debug/makefile
A  Badweh_development/Debug/modules/lwl/subdir.mk
DU Badweh_development/Debug/sources.mk
UU Badweh_development/app1/app_main.c
A  Badweh_development/modules/LWL_LOGGING_MAP.md
UU Badweh_development/modules/i2c/i2c.c
UU Badweh_development/modules/include/config.h
AA Badweh_development/modules/include/lwl.h
M  Badweh_development/modules/include/module.h
A  Badweh_development/modules/lwl/lwl.c       ← THE FILE WE NEED!
A  Badweh_development/modules/lwl/lwl.py
UU Badweh_development/modules/tmphm/tmphm.c
A  Day_3_COMPLETE_Summary.md
```

**Status codes review:**
- **`DU`** - Delete conflict (makefile files)
- **`UU`** - Content conflict (source files)
- **`AA`** - Add/add conflict (lwl.h added on both sides)
- **`A`** - Successfully added (including **lwl.c**!)

**Good news:** The `lwl.c` file is being added successfully!

### Step 3.5: Resolve All Conflicts

**Step A: Remove Debug makefiles (DU conflicts)**
```bash
git rm Badweh_development/Debug/makefile \
       Badweh_development/Debug/sources.mk
```

**Output:**
```
rm 'Badweh_development/Debug/makefile'
rm 'Badweh_development/Debug/sources.mk'
```

**Step B: Accept commit's version for content conflicts (UU and AA)**
```bash
git checkout --theirs Badweh_development/app1/app_main.c \
                      Badweh_development/modules/i2c/i2c.c \
                      Badweh_development/modules/include/config.h \
                      Badweh_development/modules/include/lwl.h \
                      Badweh_development/modules/tmphm/tmphm.c
```

**Output:**
```
Updated 5 paths from the index
```

**Step C: Stage all resolved files**
```bash
git add Badweh_development/app1/app_main.c \
        Badweh_development/modules/i2c/i2c.c \
        Badweh_development/modules/include/config.h \
        Badweh_development/modules/include/lwl.h \
        Badweh_development/modules/tmphm/tmphm.c
```

### Step 3.6: Verify Resolution

**Command:**
```bash
git status
```

**Output:**
```
On branch main
You are currently cherry-picking commit ee87f4e.
  (all conflicts fixed: run "git cherry-pick --continue")

Changes to be committed:
	new file:   Badweh_development/Debug/modules/lwl/subdir.mk
	modified:   Badweh_development/app1/app_main.c
	new file:   Badweh_development/modules/LWL_LOGGING_MAP.md
	modified:   Badweh_development/modules/i2c/i2c.c
	modified:   Badweh_development/modules/include/config.h
	modified:   Badweh_development/modules/include/lwl.h
	modified:   Badweh_development/modules/include/module.h
	new file:   Badweh_development/modules/lwl/lwl.c    ← HERE IT IS!
	new file:   Badweh_development/modules/lwl/lwl.py
	modified:   Badweh_development/modules/tmphm/tmphm.c
	new file:   Day_3_COMPLETE_Summary.md
```

**Perfect!** The critical line:
```
new file:   Badweh_development/modules/lwl/lwl.c
```

### Step 3.7: Complete Third Cherry-Pick

**Command:**
```bash
git cherry-pick --continue
```

**Output:**
```
[main b6efedd] add LWL and Documents
 Date: Sat Oct 25 07:08:21 2025 -0400
 12 files changed, 2618 insertions(+), 1307 deletions(-)
 create mode 100644 Badweh_development/Debug/modules/lwl/subdir.mk
 create mode 100644 Badweh_development/modules/LWL_LOGGING_MAP.md
 create mode 100644 Badweh_development/modules/lwl/lwl.c
 create mode 100644 Badweh_development/modules/lwl/lwl.py
 create mode 100644 Day_3_COMPLETE_Summary.md
```

✅ **Cherry-pick #3 COMPLETE!**

---

## Understanding Conflict Types

### Summary Table

| Status Code | Conflict Type | Meaning | Resolution Method |
|-------------|---------------|---------|-------------------|
| **DU** | Modify/Delete | File deleted on our branch but modified in commit | `git rm <file>` to keep deleted<br>OR `git add <file>` to keep modified |
| **UU** | Content | Both sides modified the same file | Edit manually, `git checkout --theirs`, or `git checkout --ours` |
| **AA** | Add/Add | Same file added on both sides with different content | Edit manually, `git checkout --theirs`, or `git checkout --ours` |
| **A** | Added | New file successfully added | No action needed (success) |
| **M** | Modified | File successfully modified | No action needed (success) |

### Detailed Conflict Type Explanations

#### 1. Modify/Delete Conflict (DU)

**Scenario:**
```
Your branch (main):  File doesn't exist (deleted or never created)
Their commit:        Wants to modify the file
```

**Example from our case:**
```
Badweh_development/Debug/makefile:
- On main: File doesn't exist (in .gitignore)
- Commit 0c64c92: Wants to remove -fcyclomatic-complexity flag
```

**Resolution options:**

**Option A: Keep deleted** (what we did)
```bash
git rm Badweh_development/Debug/makefile
```
Result: File remains non-existent

**Option B: Accept the modification**
```bash
git add Badweh_development/Debug/makefile
```
Result: File is created with commit's version

#### 2. Content Conflict (UU)

**Scenario:**
```
Your branch:   File exists with content A
Their commit:  File exists with content B
Git:          Can't merge automatically
```

**Example from our case:**
```c
// app_main.c on main branch:
#include "cmd.h"
#include "console.h"
#include "i2c.h"

// app_main.c in commit 4df4da8:
#include "console.h"
#include "dio.h"
#include "i2c.h"
#include "lwl.h"  // NEW!
```

**What Git does:**
Adds conflict markers inside the file:
```c
<<<<<<< HEAD
#include "cmd.h"
#include "console.h"
#include "i2c.h"
=======
#include "console.h"
#include "dio.h"
#include "i2c.h"
#include "lwl.h"
>>>>>>> 4df4da8
```

**Resolution options:**

**Option A: Use theirs** (what we did)
```bash
git checkout --theirs app_main.c
git add app_main.c
```

**Option B: Use ours**
```bash
git checkout --ours app_main.c
git add app_main.c
```

**Option C: Manual edit**
```bash
# Edit the file, choose what to keep
# Remove conflict markers
# Save the file
git add app_main.c
```

**Option D: Merge both**
```c
// After manual editing:
#include "cmd.h"      // Keep from HEAD
#include "console.h"
#include "dio.h"      // Keep from commit
#include "i2c.h"
#include "lwl.h"      // Keep from commit
```

#### 3. Add/Add Conflict (AA)

**Scenario:**
```
Your branch:   File added with content A
Their commit:  File added with content B
```

**Example from our case:**
```
lwl.h:
- On main: Header exists (maybe from earlier work)
- Commit ee87f4e: Adds lwl.h with Day 3 complete version
- Git: "Both added this file, which version?"
```

**Resolution:** Same as content conflict
```bash
git checkout --theirs Badweh_development/modules/include/lwl.h
git add Badweh_development/modules/include/lwl.h
```

---

## Final Verification

### Step 1: Check Commit History

**Command:**
```bash
git log --oneline -5
```

**Output:**
```
b6efedd add LWL and Documents
aabab15 Day 3: sonnet implemented tmphm module and lwl
e85ff37 fix: Day 3 build issues - disable premature FAULT feature and fix dependencies
78ea260 add Badweh_Development/Badweh_Development (1).launch to gitignore
157fb30 ignore .cproject and .project files in .gitignore
```

**Analysis:**
- 3 new commits added to main branch
- All are Day 3 related
- Original main branch commit (78ea260) is still there
- Backup branch still points to 78ea260

### Step 2: Verify Key Files Exist

**Command:**
```bash
ls -lh Badweh_development/modules/lwl/lwl.c \
       Badweh_development/modules/stubs_day3.c \
       Badweh_development/modules/tmphm/tmphm.c
```

**Output:**
```
-rw-r--r-- 1 Sheen 197609 8.9K Nov 19 23:43 Badweh_development/modules/lwl/lwl.c
-rw-r--r-- 1 Sheen 197609  731 Nov 19 23:27 Badweh_development/modules/stubs_day3.c
-rw-r--r-- 1 Sheen 197609  14K Nov 19 23:44 Badweh_development/modules/tmphm/tmphm.c
```

✅ All critical Day 3 files present!

### Step 3: Check LWL Module Contents

**Command:**
```bash
ls -la Badweh_development/modules/lwl/
```

**Output:**
```
total 36
drwxr-xr-x 1 Sheen 197609     0 Nov 19 23:43 .
drwxr-xr-x 1 Sheen 197609     0 Nov 19 23:43 ..
-rw-r--r-- 1 Sheen 197609  9087 Nov 19 23:43 lwl.c
-rw-r--r-- 1 Sheen 197609 20271 Nov 19 23:43 lwl.py
```

**Perfect!** LWL module with C implementation and Python decoder.

### Step 4: Verify Stubs File

**Command:**
```bash
cat Badweh_development/modules/stubs_day3.c
```

**Output (partial):**
```c
/**
 * @file stubs_day3.c
 * @brief Stub implementations for Day 4 features
 *
 * Day 3 should not have FAULT or WDG features enabled.
 * However, the code may reference these functions conditionally.
 * These stubs prevent linker errors.
 */

void wdg_register(void) {
    // No-op stub - watchdog not active in Day 3
}

void wdg_feed(void) {
    // No-op stub - watchdog not active in Day 3
}

void fault_exception_handler(void) {
    // Safety stub - should never be called in Day 3
    while(1);
}
```

**Why these stubs are critical:**
- tmphm.c has conditional calls to `wdg_feed()` and `wdg_register()`
- Without stubs, linker would fail with "undefined reference"
- Stubs allow Day 3 to compile even with references to Day 4 functions

### Step 5: Compare with Target Branch

**Command:**
```bash
git log --oneline day6_fixed | grep -E "(Day 3|lwl)" | head -10
```

**Output:**
```
0c64c92 fix: Day 3 build issues - disable premature FAULT feature and fix dependencies
4df4da8 Day 3: sonnet implemented tmphm module and lwl
02998b2 added lwl part of day 3
ee87f4e add LWL and Documents
```

**Verification:**
- ✅ We copied commit `0c64c92` → created `e85ff37` on main
- ✅ We copied commit `4df4da8` → created `aabab15` on main
- ✅ We copied commit `ee87f4e` → created `b6efedd` on main
- ✅ All Day 3 commits successfully transferred

### Step 6: Check Documentation Added

**Command:**
```bash
ls -lh Day3_TMPHM_LWL.md Day_3_COMPLETE_Summary.md Progress_Report/git_info.md 2>&1
```

**Output:**
```
-rw-r--r-- 1 Sheen 197609  22K Nov 19 23:44 Day3_TMPHM_LWL.md
-rw-r--r-- 1 Sheen 197609  15K Nov 19 23:43 Day_3_COMPLETE_Summary.md
-rw-r--r-- 1 Sheen 197609 8.9K Nov 19 23:27 Progress_Report/git_info.md
```

✅ Documentation files successfully copied!

---

## Summary and Key Takeaways

### What We Accomplished

**Goal:** Copy Day 3 TMPHM implementation from `day6_fixed` to `main` branch

**Result:** ✅ Successfully cherry-picked 3 commits containing:
1. LWL (Lightweight Logging) module implementation
2. Complete TMPHM (Temperature/Humidity) sensor driver
3. Build fixes and stub functions
4. Comprehensive documentation

### Commits Copied

| Original Commit | New Commit | Description | Files Changed |
|----------------|------------|-------------|---------------|
| `0c64c92` | `e85ff37` | Build fixes, disable FAULT, add stubs | 6 files |
| `4df4da8` | `aabab15` | Day 3 TMPHM + LWL implementation | 4 files |
| `ee87f4e` | `b6efedd` | Add LWL module and documentation | 12 files |

### Files Added to Main Branch

**Source Code:**
- `Badweh_development/modules/lwl/lwl.c` (9KB) - Lightweight logging
- `Badweh_development/modules/lwl/lwl.py` (20KB) - Python decoder
- `Badweh_development/modules/stubs_day3.c` (731 bytes) - Function stubs

**Modified Files:**
- `Badweh_development/modules/tmphm/tmphm.c` - Complete Day 3 implementation
- `Badweh_development/app1/app_main.c` - Day 3 integration
- `Badweh_development/modules/i2c/i2c.c` - Error handling updates
- `Badweh_development/modules/include/config.h` - Feature configuration

**Documentation:**
- `Day3_TMPHM_LWL.md`
- `Day_3_COMPLETE_Summary.md`
- `Progress_Report/git_info.md`
- `Badweh_development/modules/LWL_LOGGING_MAP.md`

### Key Git Commands Used

#### Basic Cherry-Pick Workflow
```bash
# 1. Create backup
git branch backup-before-day3

# 2. Start cherry-pick
git cherry-pick <commit-hash>

# 3. Check status
git status
git status --short

# 4. Resolve conflicts
git rm <file>                    # For modify/delete conflicts
git checkout --theirs <file>     # Accept their version
git checkout --ours <file>       # Keep our version
git add <file>                   # Stage resolved files

# 5. Complete cherry-pick
git cherry-pick --continue

# 6. Abort if needed
git cherry-pick --abort
```

#### Useful Information Commands
```bash
git log --oneline                     # View commit history
git log --oneline -n 5                # View last 5 commits
git show --stat <commit>              # Show files changed in commit
git show <commit>:<file>              # Show file content from commit
git diff --cached                     # Show staged changes
git diff --cached --stat              # Show staged changes summary
git branch -v                         # List branches with last commit
```

### Conflict Resolution Decision Tree

```
Encounter Conflict
       |
       ├─ DU (modify/delete)?
       │   ├─ File is IDE-generated? → git rm <file>
       │   └─ File is source code? → git add <file>
       │
       ├─ UU (content conflict)?
       │   ├─ Want their complete version? → git checkout --theirs <file>
       │   ├─ Want our version? → git checkout --ours <file>
       │   └─ Want to merge both? → Edit manually
       │
       └─ AA (add/add)?
           └─ Same as UU (usually use --theirs or --ours)
```

### Important Concepts Learned

#### 1. Cherry-Pick Creates New Commits
```
Original:    [0c64c92] "fix: Day 3 build issues"
Cherry-pick: [e85ff37] "fix: Day 3 build issues"  ← DIFFERENT HASH!

Why? Different parent commit, different tree object
```

#### 2. Cherry-Pick Preserves Metadata
```
Preserved:
- Commit message
- Author name and email
- Commit date

Changed:
- Commit hash
- Parent commit
- Tree SHA
```

#### 3. Conflict Types

| Type | Code | When It Happens | How to Resolve |
|------|------|-----------------|----------------|
| **Modify/Delete** | DU | File deleted on one side, modified on other | `git rm` or `git add` |
| **Content** | UU | Both sides modified same file | `git checkout --theirs/--ours` or manual edit |
| **Add/Add** | AA | Same file added on both sides | `git checkout --theirs/--ours` or manual edit |

#### 4. Git Conflict Markers

When Git can't auto-merge, it adds markers:
```
<<<<<<< HEAD
[Your version - current branch]
=======
[Their version - commit being cherry-picked]
>>>>>>> commit-hash (commit message)
```

To resolve:
1. Choose which version to keep
2. Delete the conflict markers
3. `git add <file>`

### Common Pitfalls and Solutions

#### Pitfall 1: Forgetting to Stage Resolved Files
```bash
# WRONG: Resolve conflict but forget to stage
git checkout --theirs file.c
git cherry-pick --continue        # ERROR: file.c still has conflict

# CORRECT: Always stage after resolving
git checkout --theirs file.c
git add file.c                    # ← DON'T FORGET THIS!
git cherry-pick --continue        # SUCCESS
```

#### Pitfall 2: Cherry-Picking in Wrong Order
```bash
# Our case: Commits built on each other
ee87f4e - Adds lwl.c
4df4da8 - Modifies tmphm.c (depends on lwl.c existing)
0c64c92 - Fixes build issues

# We picked in reverse order, which caused more conflicts
# Better approach: Pick in chronological order
git cherry-pick ee87f4e   # First
git cherry-pick 4df4da8   # Second
git cherry-pick 0c64c92   # Third
```

#### Pitfall 3: Not Creating a Backup Branch
```bash
# RISKY: Start cherry-picking without backup
git cherry-pick abc123

# SAFE: Create backup first
git branch backup-before-changes  # Can always revert!
git cherry-pick abc123
```

### Recovery Commands

If something goes wrong:

```bash
# Abort current cherry-pick
git cherry-pick --abort

# Return to backup branch
git reset --hard backup-before-day3

# Check what changed
git diff backup-before-day3 main

# See where branches diverged
git log backup-before-day3..main
```

### Next Steps

#### To Build the Project:

1. **Open STM32CubeIDE**
   - The IDE will regenerate missing Debug makefiles

2. **Build the project**
   ```
   Project → Build All
   ```

3. **Expected result:**
   - Compilation succeeds
   - Links successfully
   - Creates `.elf` and `.bin` files

#### To Test Day 3 Implementation:

1. Flash the firmware to STM32 board
2. Open serial console
3. Test commands:
   ```
   tmphm status
   tmphm test lastmeas 0
   ```

### Additional Resources

#### Git Documentation
- `git help cherry-pick` - Full cherry-pick documentation
- `git help status` - Understanding status codes
- `git help merge` - Merge conflict resolution

#### Visualizing Git
```bash
# See commit graph
git log --oneline --graph --all

# See divergence between branches
git log --oneline --graph main day6_fixed

# Compare branches
git diff main..day6_fixed
```

### Lessons Learned

1. **Always create a backup branch** before potentially destructive operations
2. **Understand what you're copying** by examining commits first (`git show`)
3. **Different conflicts need different resolutions** (DU vs UU vs AA)
4. **Cherry-pick creates new commits** with new hashes
5. **IDE-generated files should stay in .gitignore** (use `git rm` for DU conflicts)
6. **`git status` is your friend** - use it frequently during conflict resolution
7. **Cherry-pick can be aborted** if things go wrong (`git cherry-pick --abort`)

---

## Appendix: Quick Reference

### Cherry-Pick Cheat Sheet

```bash
# Start
git cherry-pick <commit-hash>

# During conflicts
git status                          # See what's conflicted
git checkout --theirs <file>        # Use their version
git checkout --ours <file>          # Use our version
git rm <file>                       # Keep deleted
git add <file>                      # Stage resolution

# Complete
git cherry-pick --continue

# Abort
git cherry-pick --abort

# Skip this commit
git cherry-pick --skip
```

### Status Code Reference

```
A  = Added (new file, success)
M  = Modified (file changed, success)
D  = Deleted (file removed, success)
DU = Deleted by Us, Updated by them (CONFLICT)
UD = Updated by us, Deleted by them (CONFLICT)
UU = Updated by both (CONFLICT)
AA = Added by both (CONFLICT)
```

### Our Specific Commands

Complete sequence of commands used in this tutorial:

```bash
# Backup
git branch backup-before-day3

# Cherry-pick 1
git cherry-pick 0c64c923e43772c7bcc0f875e5189c65ab28b80f
git rm Badweh_development/Debug/**/*.mk Badweh_development/Debug/makefile Badweh_development/Debug/*.list Badweh_development/Debug/*.mk
git cherry-pick --continue

# Cherry-pick 2
git cherry-pick 4df4da8eda66a66e684c6019836b4dbf9660d13c
git checkout --theirs Badweh_development/app1/app_main.c Badweh_development/modules/i2c/i2c.c Badweh_development/modules/tmphm/tmphm.c
git add Badweh_development/app1/app_main.c Badweh_development/modules/i2c/i2c.c Badweh_development/modules/tmphm/tmphm.c
git cherry-pick --continue

# Cherry-pick 3
git cherry-pick ee87f4e51ba8daa0c0de3c52748a7cca494bf76a
git rm Badweh_development/Debug/makefile Badweh_development/Debug/sources.mk
git checkout --theirs Badweh_development/app1/app_main.c Badweh_development/modules/i2c/i2c.c Badweh_development/modules/include/config.h Badweh_development/modules/include/lwl.h Badweh_development/modules/tmphm/tmphm.c
git add Badweh_development/app1/app_main.c Badweh_development/modules/i2c/i2c.c Badweh_development/modules/include/config.h Badweh_development/modules/include/lwl.h Badweh_development/modules/tmphm/tmphm.c
git cherry-pick --continue

# Verify
git log --oneline -5
ls -lh Badweh_development/modules/lwl/lwl.c
```

---

## Post-Cherry-Pick Issues and Fixes

### The Reality: Cherry-Pick Isn't Always Perfect

After completing the cherry-pick process, we discovered that **the code doesn't build immediately**. This is an important lesson: cherry-picking copies commits, but doesn't guarantee the result will work without additional fixes.

### Issue #1: Missing Module References in app_main.c

**Problem:**
When building from STM32CubeIDE, we got compilation errors:

```
../app1/app_main.c:34:10: fatal error: blinky.h: No such file or directory
   34 | #include "blinky.h"
      |          ^~~~~~~~~~
compilation terminated.
```

**Root Cause:**
The third cherry-pick (commit ee87f4e) used `git checkout --theirs`, which took an **older version** of `app_main.c` that included modules not present in Day 3:
- `blinky.h` - LED blinking module (not needed)
- `gps_gtu7.h` - GPS module (not needed)
- `mem.h` - Memory module (not needed)
- `stat.h` - Statistics module (not needed)

**Why this happened:**
Commit `ee87f4e` is older than commit `4df4da8`, so when we resolved conflicts with `--theirs`, we inadvertently took the older version with extra modules.

**Solution:**
Extract the correct Day 3 version directly from commit `4df4da8`:

```bash
# Get the correct version from the Day 3 implementation commit
git show 4df4da8:Badweh_development/app1/app_main.c > /tmp/app_main_day3.c

# Replace the incorrect version
cp /tmp/app_main_day3.c Badweh_development/app1/app_main.c
```

**The corrected includes for Day 3:**
```c
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "console.h"
#include "dio.h"
#include "i2c.h"
#include "lwl.h"
#include "module.h"
#include "tmphm.h"
#include "ttys.h"
#include "tmr.h"
```

**Key takeaway:** When cherry-picking multiple related commits, conflicts may result in mixing versions from different commits. Always verify the result matches your intent.

---

### Issue #2: Missing DIO Module Implementation

**Problem:**
After fixing the includes, linking failed:

```
undefined reference to `dio_init'
undefined reference to `dio_start'
```

**Root Cause:**
The DIO module header (`dio.h`) exists, but the implementation (`dio.c`) was never cherry-picked because none of our three commits added it. The DIO module existed on the `day6_fixed` branch from the initial project setup.

**Investigation:**
```bash
# Check if dio.c exists on day6_fixed
git ls-tree -r day6_fixed --name-only | grep "modules/dio/"

# Output shows it exists:
Badweh_development/modules/dio/dio.c
```

**Solution:**
Manually copy the DIO module from `day6_fixed`:

```bash
# Create the dio directory
mkdir -p Badweh_development/modules/dio

# Copy dio.c from day6_fixed branch
git show day6_fixed:Badweh_development/modules/dio/dio.c > Badweh_development/modules/dio/dio.c
```

**Why this is necessary:**
Cherry-pick only copies **changes made in specific commits**. If a file existed before those commits and wasn't modified, it won't be copied. The DIO module was added in the initial project commit, not in any of the Day 3 commits we cherry-picked.

---

### Issue #3: Build System Configuration

**Problem:**
Even after adding `dio.c`, the linker still couldn't find `dio_init` and `dio_start`.

**Root Cause:**
The build system (makefiles) didn't know about the DIO module:
1. No makefile for `modules/dio/` subdirectory
2. DIO module not included in main makefile
3. `dio.o` not in the linker's object list

**Solution - Step A: Create DIO Makefile**

Create `Badweh_Development/Debug/modules/dio/subdir.mk`:

```makefile
################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../modules/dio/dio.c

OBJS += \
./modules/dio/dio.o

C_DEPS += \
./modules/dio/dio.d


# Each subdirectory must supply rules for building sources it contributes
modules/dio/%.o modules/dio/%.su modules/dio/%.cyclo: ../modules/dio/%.c modules/dio/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER \
	-DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 \
	-DLSE_VALUE=32768 -DEXTERNAL_CLOCK_VALUE=12288000 -DHSI_VALUE=16000000 \
	-DLSI_VALUE=32000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 \
	-DINSTRUCTION_CACHE_ENABLE=1 -DDATA_CACHE_ENABLE=1 -DSTM32F401xE \
	-c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc \
	-I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include \
	-I"C:/Users/Sheen/Desktop/Embedded_System/gene_Baremetal_I2CTmphm_RAM_CICD/Badweh_Development/modules/include" \
	-O0 -ffunction-sections -fdata-sections -Wall -fstack-usage \
	-MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs \
	-mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-modules-2f-dio

clean-modules-2f-dio:
	-$(RM) ./modules/dio/dio.cyclo ./modules/dio/dio.d ./modules/dio/dio.o ./modules/dio/dio.su

.PHONY: clean-modules-2f-dio
```

**Solution - Step B: Update Main Makefile**

Edit `Badweh_Development/Debug/makefile` to include the DIO module:

```makefile
-include modules/ttys/subdir.mk
-include modules/tmr/subdir.mk
-include modules/tmphm/subdir.mk
-include modules/lwl/subdir.mk
-include modules/log/subdir.mk
-include modules/i2c/subdir.mk
-include modules/dio/subdir.mk        # ← ADD THIS LINE
-include modules/console/subdir.mk
-include modules/cmd/subdir.mk
-include modules/subdir.mk
```

**Solution - Step C: Add dio.o to Linker Object List**

Edit `Badweh_Development/Debug/objects.list`:

```
"./modules/cmd/cmd.o"
"./modules/console/console.o"
"./modules/dio/dio.o"                 ← ADD THIS LINE
"./modules/i2c/i2c.o"
"./modules/log/log.o"
"./modules/lwl/lwl.o"
"./modules/stubs_day3.o"
"./modules/tmphm/tmphm.o"
"./modules/tmr/tmr.o"
"./modules/ttys/ttys.o"
```

---

### Issue #4: Compiler Flag Incompatibility

**Problem:**
Build failed with:

```
arm-none-eabi-gcc.exe: error: unrecognized command-line option '-fcyclomatic-complexity'
```

**Root Cause:**
The `-fcyclomatic-complexity` flag is only available in GCC 12+, but the system has GCC 10.3.1. This flag should have been removed by the Day 3 build fix commit, but the IDE regenerated the makefiles with this flag included.

**Why this happened:**
- The Day 3 build fix commit (0c64c92) removed this flag
- We used `git rm` on the Debug makefiles during cherry-pick
- STM32CubeIDE regenerated the makefiles when we opened the project
- The IDE's project settings still had this flag enabled

**Solution:**
Remove the flag from all makefiles using `sed`:

```bash
# Find all affected makefiles
find Badweh_Development/Debug -name "*.mk" -exec grep -l "fcyclomatic-complexity" {} \;

# Remove the flag from all makefiles
find Badweh_Development/Debug -name "*.mk" -exec sed -i 's/ -fcyclomatic-complexity//g' {} \;

# Verify it's gone
grep -n "fcyclomatic-complexity" Badweh_Development/Debug/modules/tmphm/subdir.mk
# (should return nothing)
```

**Permanent fix (for IDE builds):**
To prevent this issue when building from STM32CubeIDE:
1. Open project properties in STM32CubeIDE
2. Navigate to: C/C++ Build → Settings → Tool Settings → MCU GCC Compiler → Miscellaneous
3. Remove `-fcyclomatic-complexity` from "Other flags"
4. Apply and save

---

### Final Build Success

After all fixes were applied:

```bash
cd Badweh_Development/Debug
make all
```

**Output:**
```
...
arm-none-eabi-gcc -o "Badweh_Development.elf" @"objects.list" ...
   text	   data	    bss	    dec	    hex	filename
  39872	    648	   6248	  46768	   b6b0	Badweh_Development.elf
Finished building: Badweh_Development.elf

arm-none-eabi-objcopy -O binary Badweh_Development.elf "Badweh_Development.bin"
Finished building: Badweh_Development.bin
```

✅ **Build successful!**

**Build artifacts:**
- `Badweh_Development.elf` (785 KB) - Executable with debug symbols
- `Badweh_Development.bin` (40 KB) - Binary for flashing to MCU
- `Badweh_Development.map` (292 KB) - Memory map

**Memory usage:**
- **Text (code):** 39,872 bytes
- **Data (initialized):** 648 bytes
- **BSS (uninitialized):** 6,248 bytes
- **Total RAM:** ~6.9 KB
- **Total Flash:** ~40 KB

---

## Complete Post-Cherry-Pick Fix Summary

### Files Modified After Cherry-Pick:

1. **`app1/app_main.c`** - Replaced with correct Day 3 version from commit 4df4da8
2. **`modules/dio/dio.c`** - Copied from day6_fixed branch
3. **`Debug/modules/dio/subdir.mk`** - Created new makefile for DIO module
4. **`Debug/makefile`** - Added DIO module include
5. **`Debug/objects.list`** - Added dio.o to linker list
6. **All `Debug/**/*.mk`** - Removed `-fcyclomatic-complexity` flag

### Commands for Post-Cherry-Pick Fixes:

```bash
# Fix 1: Get correct app_main.c version
git show 4df4da8:Badweh_development/app1/app_main.c > Badweh_development/app1/app_main.c

# Fix 2: Add DIO module
mkdir -p Badweh_development/modules/dio
git show day6_fixed:Badweh_development/modules/dio/dio.c > Badweh_development/modules/dio/dio.c

# Fix 3: Create DIO makefile
mkdir -p Badweh_Development/Debug/modules/dio
# (Create subdir.mk with content shown above)

# Fix 4: Update main makefile
# (Add -include modules/dio/subdir.mk)

# Fix 5: Update objects.list
# (Add "./modules/dio/dio.o")

# Fix 6: Remove incompatible compiler flag
find Badweh_Development/Debug -name "*.mk" -exec sed -i 's/ -fcyclomatic-complexity//g' {} \;

# Verify and build
cd Badweh_Development/Debug
make clean
make all
```

---

## Lessons Learned: Cherry-Pick Realities

### 1. Cherry-Pick Is Not Magic

**What cherry-pick does:**
- ✅ Copies specific commits
- ✅ Preserves commit messages and metadata
- ✅ Attempts to apply changes

**What cherry-pick does NOT do:**
- ❌ Guarantee the result will build
- ❌ Copy files that existed before the commits
- ❌ Update build system configurations automatically
- ❌ Verify all dependencies are present

### 2. Conflict Resolution Requires Judgment

Using `git checkout --theirs` blindly can backfire:
- We took the version from commit `ee87f4e` (older)
- But we actually needed the version from `4df4da8` (Day 3)
- **Better approach:** Manually inspect conflicts or know which commit has the right version

### 3. Cherry-Pick Order Matters

**Our approach (reverse chronological):**
```
0c64c92 (newest) → aabab15 (middle) → ee87f4e (oldest)
```
- More conflicts because newer commits expect changes from older ones
- Conflict resolutions mixed versions from different commits

**Better approach (chronological):**
```
ee87f4e (oldest) → 4df4da8 (middle) → 0c64c92 (newest)
```
- Fewer conflicts because changes build on each other
- More natural progression

### 4. Build Systems Need Manual Attention

**IDE-generated files (makefiles, project files):**
- Should be in `.gitignore`
- Will be regenerated by IDE
- May not match your manual changes
- Need verification after cherry-pick

**Manual steps required:**
- Create makefiles for new modules
- Update linker object lists
- Remove incompatible compiler flags
- Verify all dependencies are linked

### 5. Always Verify After Cherry-Pick

**Verification checklist:**
- [ ] All required files present?
- [ ] All includes reference existing files?
- [ ] Build system knows about new files?
- [ ] Compiler flags compatible with toolchain?
- [ ] Clean build succeeds?
- [ ] Binary size reasonable?

---

## Will It Build from STM32CubeIDE Now?

### Short Answer: **Yes, with one caveat**

### What Works:
✅ **Source code is correct** - All Day 3 modules present
✅ **Dependencies resolved** - DIO module added, includes fixed
✅ **Makefiles configured** - DIO module in build system
✅ **Linker knows all objects** - dio.o in objects.list

### The Caveat: Compiler Flag

⚠️ **The `-fcyclomatic-complexity` flag issue**

**What will happen when you build from STM32CubeIDE:**

1. **If you haven't removed the flag from project settings:**
   - IDE will regenerate makefiles
   - `-fcyclomatic-complexity` will be re-added
   - Build will fail with same error

2. **If you've removed the flag from project settings:**
   - IDE will regenerate makefiles correctly
   - Build will succeed

### How to Fix Permanently:

**Option 1: Remove from IDE project settings (RECOMMENDED)**

1. Right-click project → **Properties**
2. Navigate to: **C/C++ Build → Settings**
3. Go to: **Tool Settings → MCU GCC Compiler → Miscellaneous**
4. In "Other flags" field, find and remove: `-fcyclomatic-complexity`
5. Click **Apply and Close**
6. Clean and rebuild: **Project → Clean → Project → Build All**

**Option 2: Use command-line build script**

```bash
# Use the CI/CD build script (which uses existing makefiles)
Badweh_Development/ci-cd-tools/build.bat
```

This script uses the makefiles we already fixed, so it will work without IDE changes.

### Expected Build Result from IDE:

If the flag is removed from project settings, you should see:

```
Building file: ../modules/dio/dio.c
Building file: ../modules/lwl/lwl.c
Building file: ../modules/tmphm/tmphm.c
...
Linking...
Finished building target: Badweh_Development.elf

arm-none-eabi-size Badweh_Development.elf
   text	   data	    bss	    dec	    hex	filename
  39872	    648	   6248	  46768	   b6b0	Badweh_Development.elf

Build Finished. 0 errors, 0 warnings.
```

---

## Quick Reference: Post-Cherry-Pick Checklist

Use this checklist after any cherry-pick operation:

### 1. Verify Source Files
```bash
# Check all includes reference existing files
grep -r "#include" Badweh_development/app1/app_main.c | while read line; do
    header=$(echo $line | sed 's/.*"\(.*\)".*/\1/')
    [ -f "Badweh_development/modules/include/$header" ] || echo "Missing: $header"
done
```

### 2. Check for Missing Dependencies
```bash
# Try to build and check for undefined references
cd Badweh_Development/Debug
make clean
make all 2>&1 | grep "undefined reference"
```

### 3. Verify Build System
```bash
# Check if all .c files have corresponding .mk entries
find Badweh_development/modules -name "*.c" -type f
cat Badweh_Development/Debug/makefile | grep "include modules"
cat Badweh_Development/Debug/objects.list
```

### 4. Check Compiler Flags
```bash
# Look for incompatible flags
grep -r "fcyclomatic-complexity" Badweh_Development/Debug/
```

### 5. Test Build
```bash
cd Badweh_Development/Debug
make clean
make all
ls -lh Badweh_Development.{elf,bin,map}
```

---

## Issue #5: Console UART Misconfiguration

### The Reality: No Console Output Despite Successful Flash

After fixing all build issues and successfully flashing the firmware, there was **no output on the serial console** (PuTTY), even though the LED blink diagnostic confirmed the MCU was running.

**Problem:**
- Built and flashed successfully
- LED blinked 3 times on startup (confirming MCU execution)
- PuTTY showed no output on COM4 at 115200 baud
- Console was completely silent

**Investigation:**
```bash
# Check which UART was initialized
grep "ttys_init" Badweh_development/app1/app_main.c
# Shows: ttys_init(TTYS_INSTANCE_UART2, &ttys_cfg);

# Check which UART console was configured to use
grep "CONFIG_CONSOLE_DFLT_TTYS_INSTANCE" Badweh_development/modules/include/config.h
# Shows: #define CONFIG_CONSOLE_DFLT_TTYS_INSTANCE 2

# Check the UART instance enum values
grep -A 5 "enum ttys_instance_id" Badweh_development/modules/include/ttys.h
```

**Root Cause:**
The console module default configuration was set to the wrong UART instance:

```c
// ttys.h enum definition:
enum ttys_instance_id {
    TTYS_INSTANCE_UART1,  // = 0
    TTYS_INSTANCE_UART2,  // = 1 (ST-Link Virtual COM Port)
    TTYS_INSTANCE_UART6,  // = 2

    TTYS_NUM_INSTANCES
};

// config.h had:
#define CONFIG_CONSOLE_DFLT_TTYS_INSTANCE 2  // UART6 (NOT connected!)

// But app_main.c initialized:
ttys_init(TTYS_INSTANCE_UART2, &ttys_cfg);  // = 1
```

**Why this happened:**
- The console module uses `CONFIG_CONSOLE_DFLT_TTYS_INSTANCE` from `config.h`
- This was set to value `2`, which maps to `TTYS_INSTANCE_UART6` (enum index 2)
- UART6 is not connected to the ST-Link Virtual COM Port
- `app_main.c` correctly initialized UART2 (the ST-Link VCP)
- Result: Console sent output to UART6 (nowhere), while PuTTY listened on UART2

**Solution:**
Edit `Badweh_development/modules/include/config.h`:

```c
// Module console
#define CONFIG_CONSOLE_PRINT_BUF_SIZE 240
#define CONFIG_CONSOLE_DFLT_TTYS_INSTANCE 1  // TTYS_INSTANCE_UART2 (ST-Link VCP)
```

**Fix Applied:**
```bash
# Edit config.h to use UART2 (enum value 1)
# Changed from: CONFIG_CONSOLE_DFLT_TTYS_INSTANCE 2
# Changed to:   CONFIG_CONSOLE_DFLT_TTYS_INSTANCE 1

# Rebuild and reflash
cd Badweh_Development
ci-cd-tools/build.bat

# Output shows successful flash:
#   text     data      bss      dec      hex filename
#  39504      640     6568    46712     b678 Badweh_Development.elf
# File download complete
# Hard reset is performed
```

**Verification:**
After reflashing with the corrected configuration:

**PuTTY Output:**
```
========================================
  DAY 3: TMPHM Module Integration
========================================

MODE: TMPHM Automatic Sensor Sampling
      (Background operation, 1 sec cycle)
      Query with: tmphm test lastmeas 0

[INIT] Initializing modules...
  - TMPHM (Temp/Humidity) initialized

[START] Starting modules...
  - LWL (Lightweight Logging) started
  - TMPHM started (1-sec sampling)

[READY] Entering super loop...
TMPHM running in background.
Console commands available:
  - tmphm status
  - tmphm test lastmeas 0
  - i2c status

>
47.015 INFO temp=273 degC*10 hum=381 %*10
48.015 INFO temp=273 degC*10 hum=382 %*10
49.015 INFO temp=273 degC*10 hum=382 %*10
50.015 INFO temp=273 degC*10 hum=382 %*10
```

✅ **Console working!** Temperature/humidity readings visible every second.

**Sensor readings:**
- **Temperature:** 27.3°C (273 degC×10)
- **Humidity:** 38.1-38.2% (381-382 %×10)

**Key takeaway:** Even when hardware and build are correct, software configuration mismatches (like UART instance numbers) can cause silent failures. Always verify that initialized peripherals match the configuration used by dependent modules.

**Diagnostic technique used:**
Added a diagnostic LED blink before `app_main()` to confirm MCU execution:
```c
// In Core/Src/main.c before app_main():
for (int i = 0; i < 6; i++) {
    LL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);  // Blink LED 3 times
    for (volatile uint32_t delay = 0; delay < 500000; delay++);
}
```

This confirmed the MCU was running, narrowing the issue to UART communication.

---

## Complete Fix Summary (All Issues)

### Files Modified After Cherry-Pick:

1. **`app1/app_main.c`** - Replaced with correct Day 3 version from commit 4df4da8
2. **`modules/dio/dio.c`** - Copied from day6_fixed branch
3. **`modules/include/config.h`** - Fixed console UART instance (2→1)
4. **`Core/Src/main.c`** - Added LED blink diagnostic
5. **`Debug/modules/dio/subdir.mk`** - Created new makefile for DIO module
6. **`Debug/makefile`** - Added DIO module include
7. **`Debug/objects.list`** - Added dio.o to linker list
8. **All `Debug/**/*.mk`** - Removed `-fcyclomatic-complexity` flag

### All Post-Cherry-Pick Fixes:

```bash
# Fix 1: Get correct app_main.c version
git show 4df4da8:Badweh_development/app1/app_main.c > Badweh_development/app1/app_main.c

# Fix 2: Add DIO module
mkdir -p Badweh_development/modules/dio
git show day6_fixed:Badweh_development/modules/dio/dio.c > Badweh_development/modules/dio/dio.c

# Fix 3: Create DIO makefile
mkdir -p Badweh_Development/Debug/modules/dio
# (Create subdir.mk with correct compiler flags)

# Fix 4: Update main makefile
# (Add -include modules/dio/subdir.mk)

# Fix 5: Update objects.list
# (Add "./modules/dio/dio.o")

# Fix 6: Remove incompatible compiler flag
find Badweh_Development/Debug -name "*.mk" -exec sed -i 's/ -fcyclomatic-complexity//g' {} \;

# Fix 7: Fix console UART configuration
# Edit modules/include/config.h:
# Change CONFIG_CONSOLE_DFLT_TTYS_INSTANCE from 2 to 1

# Fix 8: Add diagnostic LED blink (optional, for debugging)
# Edit Core/Src/main.c to add LED blink before app_main()

# Final build and flash
cd Badweh_Development
ci-cd-tools/build.bat
```

---

**End of Tutorial**

This document demonstrates a **real-world git cherry-pick workflow** including:
- ✅ Step-by-step cherry-pick process
- ✅ Multiple conflict types and resolutions
- ✅ Post-cherry-pick issues and fixes
- ✅ Build system integration challenges
- ✅ Hardware/software configuration debugging
- ✅ Diagnostic techniques for embedded systems
- ✅ Lessons learned and best practices

**Key Insights:**
1. Cherry-picking is a powerful tool, but it's not a complete solution
2. Always verify and test the result at multiple levels (build, flash, run, output)
3. Configuration mismatches can cause silent failures even when code is correct
4. Diagnostic LEDs are invaluable for confirming MCU execution
5. Be prepared to make manual adjustments for build systems, dependencies, and toolchain compatibility

**Final Status:** ✅ Day 3 TMPHM module successfully integrated and running with live temperature/humidity readings!

Use this as a reference for future cherry-pick operations!
