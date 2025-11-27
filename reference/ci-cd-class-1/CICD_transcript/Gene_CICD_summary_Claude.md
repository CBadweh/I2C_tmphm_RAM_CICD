# Embedded CI/CD with HIL Testing - Comprehensive Course Summary

**Instructor:** Gene Schrader
**Platform:** STM32CubeIDE on Windows 10
**Hardware:** Nucleo F401RE (Device Under Test) + Blue Pill STM32 (HIL Simulation)
**Course Duration:** 6 Lessons

---

## Course Overview

### Technologies Used

**Languages & Scripting:**
- C (embedded application code)
- Python 3.x (HIL test scripts)
- Windows Batch Scripts (automation)
- Groovy (Jenkins declarative pipeline)

**Tools & Platforms:**
- STM32CubeIDE (GUI-based embedded IDE)
- Git (source code management)
- Jenkins (CI/CD build server)
- cpp-check (static code analysis)
- STM32_Programmer_CLI (flash programming)
- p-expect (Python module for stimulus-response testing)
- p-link (serial terminal from PuTTY)
- junit_xml (Python module for test reporting)

**Hardware:**
- Nucleo F401RE board (Cortex-M4, product hardware)
- Blue Pill board (STM32F103, HIL simulation hardware)
- ST-Link adapters
- USB-to-serial cable adapter (3.3V)

### Course Philosophy

- **Start Small:** "Doing just a little is better than doing nothing"
- **Incremental Approach:** Build CI/CD capabilities over time
- **GUI IDE Automation:** Demonstrates that CI/CD is possible even with GUI-based IDEs
- **Lean Jenkinsfile:** Keep pipeline scripts simple, delegate work to lower-level scripts
- **Development Culture:** Build automated testing mindset into daily workflow

---

## Lesson 1: Introduction to the Course

### Learning Objectives

**What are the learning objectives?**
- Understand the benefits and importance of CI/CD in embedded systems development
- Learn how to implement CI/CD with GUI-based IDEs (STM32CubeIDE)
- Set up hardware-in-the-loop (HIL) testing for embedded projects
- Build an automated build and delivery system on a single Windows laptop

**Why are they important in the context of embedded CI/CD?**

1. **Automation:** Saves work, prevents mistakes, reduces human error from forgotten tasks
2. **Early/Visible Feedback:** Detects bad code immediately, preventing accumulation of bugs over time
3. **Development Culture:** When integrated into workflow, developers naturally think about automated testing during feature development
4. **Industry Relevance:** CI/CD appears in many job descriptions; practical implementation skills are valuable

**How were they implemented?**
- Entire system built from scratch on Windows laptop
- Uses STM32CubeIDE (GUI-based IDE) to demonstrate that CI/CD is possible even with GUI tools
- Implements automation through scripting (Windows batch scripts)
- Uses Git for source code management
- Jenkins as build server
- Hardware-in-the-loop testing with actual embedded hardware

### System Architecture Overview

**Visual Diagram Components:**

1. **Developer Environment** (lower left):
   - STM32CubeIDE with tool chain
   - IDE project containing Git repo (working copy)
   - Product hardware (Nucleo board)
   - HIL simulation hardware (Blue Pill board)

2. **Git Server** (top):
   - Contains official copy of code (remote repo)
   - Similar to GitHub but simplified (file-based, no security complications)
   - Receives pushes from developer

3. **Build Server** (lower right):
   - Jenkins program with workspace
   - Git repo (fetched from server)
   - STM32CubeIDE tool chain (compiler/linker, IDE not actively used)
   - Product hardware for testing
   - HIL simulation hardware

4. **Software Release Store** (upper right):
   - Directory structure on laptop
   - Stores successfully built and tested image files

**Workflow:**
1. Developer makes changes, tests locally, pushes to Git server (typically master branch)
2. Git server triggers Jenkins via hook
3. Jenkins pulls code into workspace
4. Jenkins builds, performs static code analysis, programs flash, runs HIL tests
5. If all passes, copies images to software release store
6. If fails, sends email notification to developer

### Hardware and Software Used

**Hardware:**
- **Product Board:** Nucleo board with Cortex-M4 processor (STM32F401RE)
- **HIL Simulation:** STM32 Blue Pill board
- **Adapters:** ST-Link adapter (for Blue Pill), USB-to-serial cable adapter (3.3V)
- **Connections:** GPIO jumpers for testing

**Software:**
- STM32CubeIDE
- Git
- Jenkins
- Python 3.x (not 2.7)
- Windows batch scripts
- STM32 Cube Programmer (CLI tool)
- Static code analysis tools (cpp-check)

### Key Technical Details

**Prerequisites/Background:**
- Concept of source code management and Git (course explains along the way but not from scratch)
- Windows batch scripting knowledge helpful
- Basic STM32CubeIDE familiarity useful but not critical
- No deep C programming knowledge required for this course

**Challenges Addressed:**
- GUI IDE automation (tasks normally done with mouse clicks must be scripted)
- Hardware-in-the-loop testing complexity (may require simulating external hardware)
- STM32CubeIDE provides automation options explored in course

**Philosophy:**
- **Start small:** "Doing just a little is better than doing nothing"
- Example: Just running static code analysis after every change provides value
- Can always add features over time
- Build automated testing mindset as part of development culture

### Examples/Demos

**Single Developer Example:**
- Diagram shows single developer, but automation still valuable for:
  - Preventing forgotten tasks
  - Double-checking work
  - Greater confidence in software
- Even more valuable with multiple developers (unexpected interactions between work)

### Key Takeaways

- **CI/CD Benefits:** Well-known and valuable (automation, early feedback, prevents mistakes)
- **Continuous Delivery vs Deployment:** Course uses "continuous delivery" - automated process generates new releases when code goes in, made available to testers/users
- **Practical Approach:** Entire system runs on single laptop (real world would use separate servers/cloud)
- **Instructor's Experience:** Built similar system starting in 2015 for large project with many developers; CI/CD and HIL testing critical for high-rate code changes
- **Course Materials:** GitHub repos available for Nucleo board project and Blue Pill HIL simulation hardware

---

## Lesson 2: Creating STM32CubeIDE Project and Git Repo

### Learning Objectives

**What are the learning objectives?**
- Create an STM32CubeIDE project properly structured for CI/CD
- Set up a local Git repository containing the IDE project
- Understand which IDE project files should be in source code management
- Create a remote Git repository (bare repo) for official code storage
- Push local repository contents to remote repository

**Why are they important in the context of embedded CI/CD?**
- **Source Code Management:** Step one for CI/CD; essential even without CI/CD
- **Code Sharing:** Remote repo enables sharing between developers and Jenkins
- **Version Control:** Git tracks project build-up history, allows rollback if needed
- **Foundation:** Proper Git setup is prerequisite for entire CI/CD pipeline

**How were they implemented?**
- Command-line Git operations (not IDE plugin) for transparency and confidence
- Incremental commits to create useful history
- Bare repo for remote repository (standard for official code storage)
- File URLs for simplicity (everything on one laptop, no network needed)

### STM32CubeIDE Project Structure Deep Dive

**Major IDE Components:**

1. **IDE System** (common parts):
   - Eclipse IDE base (infrastructure)
   - MCU/board database (pinout, hardware info)
   - Pin and hardware configuration editor
   - Code generator (generates code from project/MCU/hardware config)
   - ST-Link debugging interface support

2. **IDE-Provided Source Libraries**:
   - **Driver Libraries:** Hardware interfacing (UART, I2C, etc.)
     - HAL (Hardware Abstraction Layer) style
     - LL (Low-Level) style
     - CMSIS (more generic, not just ST)
   - **Middleware Libraries:** RTOS, file system, TCP/IP stack
   - Copied into projects as needed based on MCU selection
   - Different versions for different MCU types

3. **MCU Tool Chains**:
   - Compiler, linker, standard C libraries
   - Standard C for printf, math, string functions
   - Come with tool chain
   - Different versions for different MCU types

**IDE Project Contents:**

Files/Folders that GO INTO Git repo:
- `.ioc` file (pin/hardware configuration settings)
- Generated initialization code
- Makefiles (generated)
- Linker script (generated)
- IDE-provided source libraries (copied in)
- Project properties/settings files (dot-prefixed, sometimes hidden)
- Application code files (hand-written)
- Build and test scripts (for CI/CD pipeline)

Files/Folders that DO NOT go into Git repo:
- Build output files (object files, map files, image files)
- These can be regenerated anytime via build

**Project Folder Structure:**
```
workspace_folder/
├── .metadata/  (workspace metadata for all projects)
└── cicd_class_1/  (project folder)
    ├── .settings/  (project settings)
    ├── app/  (application code - hand-written)
    ├── cicd_tools/  (build scripts - hand-written)
    ├── Core/  (generated)
    ├── Drivers/  (generated/IDE-supplied)
    ├── Middlewares/  (generated/IDE-supplied)
    ├── Debug/  (build output + makefiles)
    │   ├── makefiles → Git repo (needed for build)
    │   └── build output → NOT in Git repo
    ├── Release/  (same as Debug)
    ├── .cproject, .project, .mxproject  (IDE files)
    └── STM32F401RETX_FLASH.ld  (linker script)
```

### Key Technical Details

**Creating Local Git Repo - Commands:**

```bash
cd C:\path\to\project\folder
git init
git config user.name "Gene Schrader"
git config user.email "geneschrader.cicdclass@gmail.com"
# Create .gitignore file (see below)
git add .
git commit -m "IDE create project"
```

**Critical .gitignore File:**
```
# Build output files (don't store in repo)
*.o
*.d
*.su
*.map
*.elf
*.hex
*.bin
*.list

# Exception: objects.list is required for build
!objects.list

# Test results (generated during HIL testing in workspace)
test_results*.xml
```

**Why .gitignore matters:**
- Prevents build output from appearing as untracked files in `git status -u`
- Keeps repo clean and focused on source code
- If unknown file appears, investigate whether it's build output → add to .gitignore
- Exception example: `objects.list` needed for build (learned "the hard way")

**Integrating Application Code:**

Steps:
1. Create `app/` and `cicd_tools/` folders
2. Copy application code and scripts into them
3. **Unexclude from build:** Right-click app folder → Resource Configuration → Unexclude from Build (both Debug and Release)
4. Hook application into `main.c`:

```c
// In Core/Src/main.c, in the while loop:
extern void app_main(void);  // Declare function

while (1) {
    app_main();  // Call application
}
```

**Build Both Configurations:**
- Do Debug build → generates Debug makefiles
- Do Release build → generates Release makefiles
- This ensures makefiles are up-to-date before committing
- Commit sequence:
  1. Add app code and scripts → commit
  2. Build Debug and Release → commit makefiles

**Creating Remote Bare Repo - Commands:**

```bash
cd C:\Users\gene\repos  # Parent directory for repos
mkdir cicd_class_1.git  # .git suffix is convention for bare repos
cd cicd_class_1.git
git init --bare
```

**URL for this repo:**
```
file:///C:/Users/gene/repos/cicd_class_1.git
```

**Pushing to Remote Repo:**

```bash
cd C:\path\to\ide\project
git remote add origin file:///C:/Users/gene/repos/cicd_class_1.git
git push -u origin master  # -u sets default remote and branch for future pushes
```

### Useful Git Commands

**git status -u:**
- Shows modified and untracked files
- Useful to verify correct files were modified
- Catches accidental modifications
- Shows new files

**git diff:**
- Shows differences between current workspace and previous version
- `git diff <commit1> <commit2>` - differences between commits
- `git diff --name-only <commit1> <commit2>` - just show changed file names
- `git diff <commit1> <commit2> path/to/file` - show changes to specific file

**git log:**
- Shows commit history with hashes, author, date, messages

### Blue Pill Board Setup (HIL Simulation Hardware)

**Differences from Nucleo:**
- IDE doesn't know Blue Pill boards (not ST board)
- Create project for MCU type (not specific board)
- More manual hardware configuration required

**Critical Configurations:**
1. **Enable Debug:** CRITICAL - without this, may have problems flashing board in future
2. **Configure UART:** For console serial port
3. **Clock Configuration:** Modify to get faster clock (72 MHz vs default slow clock)
   - Enable 8 MHz crystal
   - Adjust clock configuration
   - Should see 72 MHz output frequencies

### Examples/Demos

**Demo: Creating Repo in Steps**

Shown in command window:
1. Create .gitignore file
2. `git init` - creates repo
3. `git status -u` - shows untracked files
4. `git config` - set username and email
5. `git add .` - stage all files
6. `git commit -m "IDE create project"` - commit
7. Add app and cicd_tools folders
8. `git status -u` - see new files and modified settings
9. `git add .` and `git commit -m "Add app and cicd scripts"`
10. Build Debug and Release
11. `git status -u` - see new makefiles
12. `git add .` and `git commit -m "Debug and release builds"`

**Demo: Git History and Diff**

```bash
git log  # Shows 3 commits with hashes

# Diff between first and second commit (file names only)
git diff e6234cb 107d715 --name-only

# Diff for specific file
git diff e6234cb 107d715 Core/Src/main.c
# Shows added lines for app_main() call
```

### Key Takeaways

**IDE Plugin vs Command Line:**
- STM32CubeIDE has Git plugin, but instructor chose command line because:
  - Important to know Git at command line
  - Course is about moving from GUI to command line
  - More confidence in what's happening
  - GUI experience: often went back to command line anyway

**Incremental Commits - Pros/Cons:**

Pros:
- Creates history of how project was built
- Can back out last commit if mistake made halfway through
- Learning experience

Cons:
- Slightly larger repo (not by much)

Note: Backing out commits happens in software, but not done often due to complications - usually just fix mistake and make another commit

**Bare Repo Characteristics:**
- Normal method for sharing code (like GitHub)
- Used for official copy
- Remote (contrast to local repo in IDE)
- Cannot directly modify files (must push changes from local repo)
- Jenkins obtains code from this repo

**File URLs:**
- Used for simplicity (everything on laptop)
- Normally would use SSH or HTTPS URLs
- No network needed with file URLs

---

## Lesson 3: Automation of STM32CubeIDE Tasks

### Learning Objectives

**What are the learning objectives?**
- Identify which IDE tasks must be automated for CI/CD
- Learn how to automate building flash images from command line
- Learn how to automate programming flash memory from scripts
- Understand make-based builds vs headless IDE builds
- Handle makefile issues (full path names problem)

**Why are they important in the context of embedded CI/CD?**
- **Core Automation Requirement:** Building and flash programming are constantly repeated tasks that MUST be automated for CI/CD
- **Independence from GUI:** Scripts allow Jenkins to perform builds without GUI interaction
- **Repeatability:** Automated scripts ensure consistent build process every time
- **Foundation for Pipeline:** These automation scripts become the building blocks Jenkins will invoke

**How were they implemented?**
- Used make (option 2) instead of headless IDE (option 1) for greater independence
- Created Windows batch scripts that set environment and invoke make
- Used STM32_Programmer_CLI for flash programming
- IDE provides environment variables that make script creation easy

### Tasks Analysis - What to Automate?

**Manual Tasks (Not Automated):**
- Creating project
- Configuring hardware
- Design work
- These are design tasks, nothing to do with CI/CD automation

**Must Automate for CI/CD:**
- **Building images** - constantly modifying code that needs building
- **Programming flash** - need to load images to hardware for testing

**Optional/Not Automated in This Course:**
- **Regeneration of IDE-supplied code** - done after hardware config changes
  - Not often needed
  - Easy to regenerate in IDE when making config changes
  - IDE often reminds you to regenerate

- **Creating/updating makefiles** - can happen when adding .c files or changing compiler settings
  - Would be nice to automate, but not done in this course
  - Workaround: Developer workflow accommodates this

### Developer Workflow to Accommodate Makefile Updates

**Recommended Workflow:**
1. Make code changes in IDE project
2. Build first to fix compiler problems
3. Test and debug using IDE
4. Iterate: make more changes, test/debug (loop as needed)
5. When code is ready to push to Git for CI/CD:
   - **Do final IDE builds for BOTH Debug AND Release** (ensures makefiles up-to-date)
   - May be no makefile changes, but do this step to be certain
6. Git commit file changes (both code AND makefiles)
7. Push commit to remote repo

**Result:** Workable system even if not ideal

### Build Automation - Two Options

**Option 1: Headless IDE Build**
- Run STM32CubeIDE in "headless" way via command line
- GUI doesn't appear on screen
- IDE just does the work
- Example in file installed with IDE
- **Instructor's experience:** Had issues, real doubts about reliability
- **Decision:** Did NOT use this option

**Option 2: Use Make (Selected)**
- IDE itself uses make for builds
- Key: IDE generates and maintains makefiles
- **Advantages:**
  - Uses standard make tool
  - No dependence on IDE
  - No fighting with IDE
  - Move away from IDE in project
- **Disadvantages:**
  - Depend on IDE to update makefiles when needed (discussed in workflow above)

### Build Script Creation

**Getting Information from IDE:**

Path: Project Properties → C/C++ Build → Environment

Key variables to extract:
1. **CWD** (Current Working Directory): Where to perform build
2. **PATH**: Updated path including tool chain directories

**IDE Build Command:**
Shown in Console after build:
```bash
make -j4 all
```
- `-j4`: Run up to 4 steps in parallel (instructor has dual-core with hyper-threading)
- `all`: Make target

**Trivial Build Script (Hard-Coded):**

```batch
@echo off
setlocal

set CWD=C:\Users\gene\workspace\cicd_class_1\Debug
set PATH=C:\...\very\long\path\...(5 lines long)

cd /d %CWD%
make -j4 all
```

Simple script - everything hard-coded, but demonstrates the concept.

**Production Build Script for Jenkins:**

More complex because:
- No hard-coding - information passed as arguments
- One additional important function: Build ID injection

**Build ID Injection:**
```batch
REM Create version.h with build ID
echo #ifndef VERSION_H > version.h
echo #define VERSION_H >> version.h
echo #define BUILD_VERSION "%BUILD_ID%" >> version.h
echo #endif >> version.h
```

Why important:
- Console command can print software version
- Ensures testing the RIGHT version
- Prevents testing wrong software (like Ring doorbell story in Lesson 4)

**Build Script Usage:**
```batch
build.bat <config> <target> <project_path> <toolchain_path> <build_id>
```
- config: Debug or Release
- target: clean or all
- Other args: paths and version info passed in (not hard-coded)

### Flash Programming Automation

**Tool: STM32_Programmer_CLI**
- Comes with STM32 Cube Programmer (GUI tool - must download/install)
- CLI tool does everything needed

**Required Information:**
1. **Image file:** Flash image to program
2. **Flash address:** Where to start programming
   - For all STM32 Cortex-M MCUs instructor used: `0x08000000` (start of flash)
   - Can verify in datasheet or linker script
3. **ST-Link serial number:** Which interface to use
   - Optional if only one ST-Link connected
   - Required for multiple boards
   - Get with: `STM32_Programmer_CLI -l` (lists connected ST-Links)

**Trivial Flash Script (Hard-Coded):**

```batch
@echo off
setlocal

set CLI=C:\path\to\STM32_Programmer_CLI.exe
set STLINK=066DFF535150898367092722
set IMAGE=C:\path\to\project\Debug\cicd_class_1.elf

%CLI% -c port=SWD sn=%STLINK% -d %IMAGE% 0x08000000 -hardRst
```

Command breakdown:
- `-c port=SWD sn=%STLINK%`: Connect via SWD port with specific serial number
- `-d %IMAGE% 0x08000000`: Download (program) image starting at address
- `-hardRst`: Do hardware reset when done

**Production Script:**
- Similar but doesn't hard-code values
- Located at: `cicd_tools/flash.bat`

### Makefile Issue and Solution

**Problem Discovered:**
- Scanning through makefiles found full path name for linker script
- Example line in makefile:
```makefile
LDSCRIPT = C:\Users\gene\workspace\cicd_class_1\STM32F401RETX_FLASH.ld
```
- Problem: This path references IDE project folder
- OK for IDE builds, but NOT OK for Jenkins builds
- **Jenkins requirement:** Only files pulled from Git into Jenkins workspace can be used
- Build cannot reference IDE project folder files

**Solution: Change in Project Properties**

Path: Project Properties → (navigate through to linker script setting)

Change from:
```
${workspace_loc:/${ProjName}/STM32F401RETX_FLASH.ld}
```

To relative path:
```
../STM32F401RETX_FLASH.ld
```

From CWD (current working directory where build happens), this relative path works.

**Alternative Solution (discussed):**
- Pre-build script executed by Jenkins
- Script edits makefile copy in Jenkins workspace
- Changes full path to relative path
- More complex, not used in course

### Examples/Demos

**Demo: Build and Flash from Command Line**

Location: IDE project top level folder

Scripts for IDE project use:
- `build_i.bat` - "i" for IDE project
- `flash_i.bat` - "i" for IDE project

These are simpler versions for manual command-line use when working in IDE.

**Demo sequence:**
```batch
cd C:\Users\gene\workspace\cicd_class_1\cicd_tools

REM Clean debug build
build_i.bat Debug clean
REM Output: Removes object files, images, etc. (fast)

REM Build debug all
build_i.bat Debug all
REM Output: Compiles everything, takes time

REM Flash debug image
flash_i.bat Debug
REM Output: Programs flash quickly, board ready for testing
```

**What the demo shows:**
- Build automation works from command line
- No IDE GUI needed
- Scripts Jenkins will use
- Fast, repeatable process

### Key Takeaways

**Build Automation:**
- Make-based approach provides independence from IDE
- IDE-generated makefiles are key enabler
- Developer workflow ensures makefiles stay current
- Build ID injection is critical for testing verification

**Flash Programming:**
- STM32_Programmer_CLI provides simple command-line interface
- ST-Link serial numbers allow multi-board setups
- Hardware reset ensures clean start after programming

**Makefile Issues:**
- Full paths in makefiles can break Jenkins builds
- Relative paths solve the problem
- Alternative: pre-build script to fix paths (more complex)

**Changes Requiring IDE Builds to Update Makefiles:**
- Adding/removing application source files
- Changing compiler options (e.g., `-D` preprocessor defines)
- Switching between HAL and LL libraries
- Best practice: Always do both Debug and Release IDE builds before committing

**Build Failure Example:**
- Code changes pushed, Debug build succeeds, Release build fails in Jenkins
- Likely cause: Only did IDE build for Debug, not Release
- Makefiles only updated for Debug folder

---

## Lesson 4: Static Code Analysis and HIL Testing

### Learning Objectives

**What are the learning objectives?**
- Understand static code analysis as a CI/CD pipeline component
- Design and implement hardware-in-the-loop (HIL) testing for embedded systems
- Create Python test scripts using p-expect for stimulus-response testing
- Use JUnit XML format for test result reporting
- Verify build ID during automated testing

**Why are they important in the context of embedded CI/CD?**
- **Static Code Analysis:** Automatically finds potential bugs without running code; provides quick feedback to developers while code is fresh
- **HIL Testing:** Tests actual hardware behavior, not just simulation; prevents "dead on arrival" loads; ensures software actually boots and runs
- **Build ID Verification:** Prevents testing wrong software version (critical lesson from Ring doorbell story)
- **Test Reporting:** JUnit XML enables Jenkins to display test results in GUI

**How were they implemented?**
- Static code analysis: cpp-check tool, run only on application code (not IDE/third-party code)
- HIL test script: Python with p-expect module, controls two boards via serial links
- Test hardware: Nucleo (DUT) + Blue Pill (HIL simulation) connected via GPIO
- Serial communication: p-link serial terminal programs
- Results: JUnit XML format for Jenkins integration

### Static Code Analysis

**What is Static Code Analysis?**
- Examining code WITHOUT running it
- When done by tool → static code analysis
- When done by person → code review
- Normally done to find potential bugs

**Why in CI/CD Pipeline?**
- Always runs when code changes (no one has to remember)
- Quick feedback to developer
- Code is fresh in developer's mind, easier to fix

**Tool Choice: cpp-check**
- Free version available (also licensed premium version)
- Easy to run from script
- Alternative mentioned: Coverity (licensed, "incredible job at analyzing code")

**Suppressing False Positives:**
- Can put special comments before "problem" code
- Example:
```c
// Fault handling code doing address manipulations
// cppcheck-suppress comparePointers
pointer_manipulation_code_here();
```
- Only use when necessary
- Sometimes better to modify code to make cpp-check happy

**Scope Decision:**
- Course: Run ONLY on application code, not IDE-provided or third-party code
- Rationale:
  - Third-party code: Only look for serious problems
  - Not interested in fixing trivial things in third-party code
  - Modifying third-party code creates software management burden

**Pipeline Behavior on cpp-check Failure:**
- Does NOT stop pipeline
- Marks build as "unstable" (can see something not quite right)
- Continues to next stages
- Allows flexibility in handling minor issues

### Hardware-in-the-Loop Testing

**Types of Automated Testing:**

Common types:
- **Unit Tests:** Test individual functions/modules
- **Integration Tests:** Higher-level tests with hardware simulation
- **HIL Tests:** Test with actual product hardware

Unit/Integration characteristics:
- Involve test stubs/harnesses (additional software)
- Usually run on build host (not embedded target)
- Product code may need special builds for testing
- Often see `#ifdef` to handle special builds

**Why Emphasize HIL in This Course?**
- Tests actual hardware behavior
- Even small amount of HIL testing is valuable
- More types of testing = better (decision is where to spend time)

**Minimum Valuable HIL Test:**
- Just loading software and running it
- Verify it comes up (perhaps using console)
- **Prevents "dead on arrival" loads** - embarrassing when last 5 loads won't even boot
- Important for continuous delivery

### Build ID Verification - The Ring Doorbell Story

**Why Verify Build ID?**
Critical to ensure testing the CORRECT software version.

**Real-World Example:**
- Ring doorbell first product shipped right before Christmas
- Immediately got complaints about video quality
- Could reproduce in lab
- Investigation found: Software changes made right before ship
- Testing was "all okay"
- **BUT: Wasn't actually testing new software** (setup problem)
- **Shipped software that was NEVER tested**
- Gut-wrenching discovery for team
- Lucky: Could fix problem on cloud side (happy ending)
- Otherwise: Ring doorbell might not have succeeded as product

**Lesson:** ALWAYS verify build ID as part of testing.

### HIL Test Setup

**System Diagram:**

```
Build Server (laptop)
├── base_hilt.py (Python test script)
│   ├── Can run from Jenkins pipeline
│   └── Can run manually from command line
├── p-link instances (2)
│   ├── Serial terminal programs
│   └── Allow test script to communicate with boards
├── Nucleo board (Device Under Test - DUT)
│   ├── Product hardware
│   └── USB connection (ST-Link: serial + debug)
├── Blue Pill board (HIL Simulation Hardware)
│   ├── Simulates external hardware (GPIO)
│   ├── USB-to-serial adapter (3.3V)
│   └── ST-Link adapter (not needed for CI/CD, just development)
└── test_results*.xml (JUnit XML format)
    └── Jenkins reads and displays results
```

**Physical Connections:**

Laptop to Blue Pill:
- USB-to-serial adapter (3.3V - IMPORTANT)
- ST-Link debug interface (for development only, not CI/CD)

Laptop to Nucleo:
- USB cable for ST-Link (provides serial + debug interface)

Between boards:
- GPIO connection: Port B Pin 9 (Blue Pill to Nucleo)
- Ground connection (tie grounds together)
- Loopback on Nucleo: Allows some testing even without Blue Pill

### Application Code - GPIO Command Processor

**Purpose:**
- Created simple GPIO read/write application via serial console
- Easy to understand for demonstration purposes
- Same software runs on both DUT and HIL simulation hardware

**Console Commands Supported:**

```
config <port> <pin> <direction>   # Configure pin (0=input, 1=output)
read <port> <pin>                 # Read pin value
write <port> <pin> <value>        # Write pin value (0 or 1)
reset                             # Reset the board
version                           # Get software version
```

**Software Architecture:**
- Simple super loop
- Uses STM32 HAL UART library
- Made portable to other STM32 MCUs

**Code Structure:**

File: `app/gpioapp/appmain.c`

Main function contains loop:
```c
void app_main(void) {
    while (1) {
        get_command_line();           // Get input
        parse_command_to_tokens();     // Parse
        process_command();             // Execute
    }
}
```

Read/write implementation:
```c
// Read command
value = HAL_GPIO_ReadPin(port, pin);

// Write command
HAL_GPIO_WritePin(port, pin, value);
```

### Python Test Script

**Tool Choice:**
- Python (instructor's favorite after C/C++)
- p-expect module: Stimulus-response testing
- Based on popular "expect" tool (instructor used in 1990s)
- p-link: Serial terminal program from PuTTY
- junit_xml module: Makes JUnit XML format easy

**Script Location:**
`cicd_tools/base_hilt.py`

**Test Structure - Data-Driven:**

When lots of repetition, create data structures describing tests as series of steps.

```python
# Example GPIO test
test_steps = [
    {
        'device': 'dut',
        'command': 'config B 9 1',     # Configure DUT pin B9 as output
        'expect': 'OK'
    },
    {
        'device': 'sim',
        'command': 'config B 9 0',     # Configure SIM pin B9 as input
        'expect': 'OK'
    },
    {
        'device': 'dut',
        'command': 'write B 9 1',      # Write 1 to DUT
        'expect': 'OK'
    },
    {
        'device': 'sim',
        'command': 'read B 9',         # Read from SIM
        'expect': '1'                  # Expect to get 1 (pins connected)
    },
    {
        'device': 'dut',
        'command': 'write B 9 0',      # Write 0 to DUT
        'expect': 'OK'
    },
    {
        'device': 'sim',
        'command': 'read B 9',         # Read from SIM
        'expect': '0'                  # Expect to get 0
    }
]
```

Each step specifies:
- Device to talk to (DUT or SIM)
- Command to send
- Expected response

**Key Test Functions:**

1. **Console Prompt Test** (always first):
```python
def test_console_prompt():
    send_line('')              # Send empty line
    expect_prompt()            # Check for prompt
    # If no prompt, something really wrong with software
```

2. **Version Check Test** (critical):
```python
def test_version(tver):    # tver = target version from Jenkins
    send('version')
    expect(f'version={tver}')  # Must match expected version
    # If doesn't match, testing WRONG version, fail test
```

**How Version String Gets to Script:**
- Input parameter to Python script
- Passed from Jenkins pipeline
- If mismatch, test fails immediately

### Examples/Demos

**Demo 1: Successful HIL Test Run**

```bash
cd cicd_tools
python3 base_hilt.py --debug --tver "BuildID_12345"
```

Output:
- Debug log printed
- Runs all tests
- Writes JUnit XML files
- Echoes XML contents to screen
- Summary: "19 tests, 0 disabled, 0 errors, 0 failures"

**What Logging Shows:**
- What script is sending to DUT and SIM
- What it expects in response
- Example: Configure pins, write values, read values

**Demo 2: Failed Test (Removed GPIO Jumper + Wrong Version)**

```bash
cd cicd_tools
python3 base_hilt.py --debug --tver "BuildID_12345_XXX"  # Wrong version
# Also physically removed GPIO jumper
```

Output:
- Takes longer (timeouts when expectations not met)
- Summary: "19 tests, 5 failures"
- Failures shown in XML:
  - Version test failed (pattern not matched)
  - 4 GPIO tests failed (jumper removed, pattern not matched)

**What Demo Shows:**
- Test script catches version mismatches
- Hardware failures detected (missing jumper)
- JUnit XML captures failure details
- Jenkins will display these results

### Key Takeaways

**Static Code Analysis:**
- Examines code without running it
- Finds potential bugs compiler might miss
- Good for CI/CD pipeline component
- cpp-check is free and easy to use
- Can suppress false positives with special comments
- Decision: What code to analyze (application only vs all code)

**HIL Testing Philosophy:**
- Even small amount is valuable (just booting software)
- Prevents dead-on-arrival loads
- More testing types = better
- Doesn't have to be 100% black-box to be useful

**Build ID Verification:**
- CRITICAL to verify correct version being tested
- Ring doorbell story: shipped untested software, near disaster
- Always verify as part of test script

**Test Automation:**
- Python + p-expect good for stimulus-response testing
- p-link provides serial terminal capability
- Data-driven tests reduce repetition
- JUnit XML enables Jenkins integration

---

## Lesson 5: Setting up Jenkins

### Learning Objectives

**What are the learning objectives?**
- Install and configure Jenkins on Windows
- Understand Jenkins architecture and pipeline concepts
- Create a declarative Jenkins pipeline using Jenkinsfile
- Configure email notifications for build failures
- Set up Git web hooks to trigger builds automatically
- Understand Jenkins parameterized builds
- Read and interpret Jenkins build results

**Why are they important in the context of embedded CI/CD?**
- **Jenkins as Automation Hub:** Pulls together all automation scripts into coordinated pipeline
- **Automated Triggers:** Web hooks enable hands-free build process when code pushed
- **Visibility:** Dashboard and email notifications provide immediate feedback
- **Repeatability:** Pipeline ensures same steps executed every time
- **Official Storage:** Jenkins stores build results and test reports

**How were they implemented?**
- Declarative pipeline (simpler, more modern than scripted)
- Jenkinsfile stored in Git repo with source code
- Generic Web Hook Trigger plugin for Git integration
- Extended Email Notification plugin for failure alerts
- Parameterized build for hardware configuration (COM ports, ST-Link serial numbers)

### Jenkins Introduction

**Simple View:**
"Sophisticated script runner and data store specialized for DevOps"

**Plugin-Based Architecture:**
Good:
- Third parties easily add features
- Makes Jenkins powerful

Bad:
- More poking around to figure out usage
- Documentation sometimes inconsistent
- Popular plugins have lots of web discussion

**Project Types (Jobs):**

1. **Freestyle Projects:**
   - Instructor used in past work
   - Worked fine
   - Some pipeline support, but do lot yourself

2. **Pipeline Projects** (Used in this course):
   - Supports pipeline concepts completely
   - More modern approach
   - Recommended

**Pipeline Styles:**

1. **Scripted Pipelines:**
   - Original style
   - Powerful but more complex scripting language
   - First keyword: `node`

2. **Declarative Pipelines** (Used in this course):
   - Simpler syntax
   - More popular
   - Can contain pieces of advanced script language if needed
   - First keyword: `pipeline`

**Jenkinsfile:**
- Pipeline script stored in file named "Jenkinsfile"
- Can store on Jenkins server OR with product source code in Git repo
- **Course approach:** Store in Git repo
- **Benefits of Git storage:**
  - CI/CD tools under same source control as software
  - Coordinated changes easier
  - History maintained together

**Lean Jenkinsfile Style:**
- Keep Jenkinsfile lean (doesn't contain much logic)
- Just calls lower-level scripts to do work
- **Advantages:**
  - More independent of Jenkins
  - More logic in your own scripts
  - Less Jenkins script language to learn
  - Simpler Jenkins usage

### Jenkins Installation

**Important Notes:**
- Not detailed installation walkthrough
- Hints based on instructor's experience
- Using Windows 10 (Mac/Linux may differ)
- Read Jenkins installation instructions carefully

**Java JDK Requirement:**

Jenkins requires Java JDK.

**Instructor's Installation Issues:**
- First attempt: Plugin installation errors relating to certificates/security
- Suspected problem: Java JDK (didn't understand open Java JDK options)
- **Solution:** Started over with specific JDK known to work

**Recommended JDK:**
- Specific version known to work based on web community
- Did NOT add to PATH (not necessary)
- **MUST set environment variable:** `JAVA_HOME`
  - How Jenkins finds Java JDK

**Windows User for Jenkins Service:**

Jenkins runs as background service, needs Windows user.

Options:
1. Create new user for Jenkins
2. Use own Windows login user ID (instructor's choice)

**Security Configuration Required:**

Even using own user ID, must modify Windows security to allow running Jenkins service.

Tool: Local Security Policy app (Windows)

Path:
1. Open Local Security Policy
2. Navigate to specific privilege
3. Add user to allowed list

**Jenkins HTTP Interface:**

Default URL:
```
http://localhost:8080
```
- localhost: On your laptop/browser machine
- Port 8080: Default (configurable)
- Jenkins runs as service

**Required Plugins:**

List to install/verify:
- Generic Web Hook Trigger
- Extended Email Notification
- JUnit
- (Others may be installed by default)

### Email Configuration

**Global Level Setup:**
- Configured at Jenkins global level (not per-pipeline)
- Pipeline makes decision to send email
- Global level defines server parameters

**Configuration Path:**

Dashboard → Manage Jenkins → Configure System → Extended Email Notification

**SMTP Server Configuration:**

For Gmail:
```
SMTP server: smtp.gmail.com
Port: 465 (or 587)
```

**Authentication:**

Used username/password (not very secure):
- Gmail account for class
- **Required Google Account Setting:**
  - Manage Google Account → Security
  - Turn ON "Less secure app access"
  - NOTE: Google may disallow this in future
  - **Should use:** Certificates (instructor "too lazy")

**Default Email Settings:**

```
Default Recipients: geneschrader.cicdclass@gmail.com
Default Subject: $PROJECT_NAME - Build # $BUILD_NUMBER - $BUILD_STATUS!
Default Body: $DEFAULT_CONTENT
```

**Jenkins Global Variables:**
- `$PROJECT_NAME`, `$BUILD_NUMBER`, `$BUILD_STATUS`, etc.
- Documented in Jenkins
- Provide useful information in emails

### Pipeline Configuration

**Creating New Pipeline:**

Dashboard → New Item → Pipeline

**Parameterized Build:**

Parameters for hardware configuration:

```
Name: COM_DUT
Default: COM13
Description: COM port for device under test

Name: COM_SIM
Default: COM15
Description: COM port for HIL simulation hardware

Name: STLINK_SN
Default: 066DFF535150898367092722
Description: ST-Link serial number for flash programming
```

**Why Parameters?**
- Match actual hardware setup
- Default values correct for instructor's setup
- No need to set manually each time

**Build Triggers:**

Configure: Generic Web Hook Trigger
- Enables web hook from Git server
- Git hook notifies Jenkins when new software pushed
- **Token:** cicd_class_1
  - Git trigger includes this token
  - Allows Jenkins to identify which project/job to build

**Pipeline Script Source:**

Option 1: Enter script directly in form
Option 2: Get from SCM (Source Code Management) - **USED IN COURSE**

**SCM Configuration:**
```
SCM: Git
Repository URL: file:///C:/Users/gene/repos/cicd_class_1.git
Credentials: (none needed for file repo, would need for SSH/HTTPS)
Branch: master
Script Path: Jenkinsfile
```

### Jenkinsfile - Declarative Pipeline Script

**Location:** Root of IDE workspace, part of Git repo

**Identifying Declarative:**
```groovy
pipeline {    // First keyword = declarative
    ...
}
```

**Jenkinsfile Structure:**

```groovy
pipeline {
    // Environment variables
    environment {
        TOOL_DIR = "${WORKSPACE}/cicd_tools"
    }

    // Stages
    stages {
        stage('Build') {
            steps {
                // Build debug
                bat "build.bat Debug all ${WORKSPACE} ..."
                // Build release
                bat "build.bat Release all ${WORKSPACE} ..."
            }
        }

        stage('Static Analysis') {
            steps {
                catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
                    bat "static_analysis.bat"
                }
            }
        }

        stage('Flash Debug') {
            steps {
                bat "flash.bat Debug ${STLINK_SN} ..."
            }
        }

        stage('Test Debug') {
            steps {
                bat "python3 base_hilt.py --tver ${BUILD_ID} --com_dut ${COM_DUT} ..."
                junit 'test_results*.xml'
                bat "check_test_results.bat test_results*.xml"
            }
        }

        stage('Flash Release') {
            steps {
                bat "flash.bat Release ${STLINK_SN} ..."
            }
        }

        stage('Test Release') {
            steps {
                bat "python3 base_hilt.py --tver ${BUILD_ID} ..."
                junit 'test_results*.xml'
                bat "check_test_results.bat test_results*.xml"
            }
        }
    }

    // Post-build actions
    post {
        success {
            bat "deliver.bat"
        }
        unsuccessful {
            emailext subject: '...',
                     body: '...',
                     to: '...'
        }
    }
}
```

**Key Components Explained:**

**1. Environment Section:**
```groovy
environment {
    TOOL_DIR = "${WORKSPACE}/cicd_tools"
}
```
- Declare variables
- `WORKSPACE`: Jenkins-defined variable, top directory of workspace
- Before pipeline runs, Jenkins already fetched repo/code

**2. Build Stage:**
```groovy
bat "build.bat Debug all ${WORKSPACE} ..."
bat "build.bat Release all ${WORKSPACE} ..."
```
- `bat`: Windows batch file step
- Builds Debug, then Release
- Passes arguments (not hard-coded)
- If script fails (build error), stage fails → entire build fails by default

**3. Static Analysis Stage:**
```groovy
catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
    bat "static_analysis.bat"
}
```
- Runs `static_analysis.bat` (invokes cpp-check)
- If fails: Mark stage as FAILED, but build as UNSTABLE
- Doesn't stop pipeline (continues)

**4. Test Stage:**
```groovy
bat "python3 base_hilt.py --tver ${BUILD_ID} ..."
junit 'test_results*.xml'
bat "check_test_results.bat test_results*.xml"
```
- Runs HIL test script
- `junit` step: Reads XML results, makes available for display
- `check_test_results.bat`: Examines XML, fails if any test failures

**5. Post-Build Section:**

```groovy
post {
    success {
        bat "deliver.bat"
    }
    unsuccessful {
        emailext subject: '...', body: '...', to: '...'
    }
}
```

**Success:**
- `deliver.bat`: Packages image files + other files, copies to delivery area

**Unsuccessful:**
- Sends email notification

### Git Web Hook Setup

**Purpose:**
Trigger pipeline automatically when code pushed to official Git repo.

**Git Hooks:**
- Scripts that execute when particular Git action occurs
- Server-side hooks (official repo) - **USED HERE**

**Post-Update Hook:**
- Server-side hook
- Runs AFTER new software pushed into repo
- **Magic filename:** `post-update` (no extension)

**Hook Script Location:**
```
C:/Users/gene/repos/cicd_class_1.git/hooks/post-update
```

**Hook Script Content:**
```bash
#!/bin/bash
curl -u gene:password http://localhost:8080/generic-webhook-trigger/invoke?token=cicd_class_1
```

Breakdown:
- `#!/bin/bash`: Required
- `curl`: HTTP transaction tool
- `-u gene:password`: Jenkins username/password
- URL: Jenkins endpoint for webhook
  - `localhost:8080`: Jenkins server
  - `/generic-webhook-trigger/invoke`: Plugin-required path
  - `?token=cicd_class_1`: Token to identify which project to run

### Jenkins Dashboard

**Build History Table:**

Columns = Stages:
- Checkout (from SCM)
- Build
- Static Analysis
- Flash Debug
- Test Debug
- Flash Release
- Test Release
- Post-Build Actions

Rows = Individual Builds:
- Colors indicate status:
  - Green: Success
  - Yellow: Unstable
  - Red: Failed
- Can see exactly where build failed

### Examples/Demos

**Demo: Code Change Triggering Build**

**Scenario:** Add bad code, push to Git, watch Jenkins automatically build and catch problem.

**Code Change:**
```c
// In main.c
void dummy(void) {
    int *ptr = NULL;    // Create null pointer
    *ptr = 5;           // Use it immediately - BAD!
}
```

Compiler doesn't notice, but static code analysis will.

**Git Operations:**
```bash
cd C:/Users/gene/workspace/cicd_class_1

git status
git diff
git add Core/Src/main.c
git commit -m "Bad dummy function"
git push    # This triggers Jenkins!
```

**Jenkins Response:**
- Dashboard shows new build starting immediately
- Pulls code from Git
- Starts building
- Build succeeded
- Static code analysis FAILED (yellow = unstable)

**Build Result:**
- cpp-check found: "Null pointer dereference"
- Compiler didn't notice, static code checker did
- Email notification sent

### Key Takeaways

**Jenkins Architecture:**
- Sophisticated script runner and data store for DevOps
- Plugin-based (good: extensible; bad: sometimes inconsistent docs)
- Declarative pipelines simpler and more popular than scripted
- Lean Jenkinsfile approach: Just call lower-level scripts

**Installation Considerations:**
- Java JDK required, `JAVA_HOME` must be set
- Windows user for Jenkins service
- Security policy modification may be needed
- Plugin installation critical

**Pipeline Features:**
- Parameterized builds for hardware configuration flexibility
- Environment variables for paths and settings
- Stages with clear purposes
- Error handling: catchError allows continuing despite failures
- Post-build actions: success vs unsuccessful paths

**Git Integration:**
- Web hook triggers automatic builds on push
- Post-update hook uses curl to call Jenkins API
- Token identifies which project to build
- Jenkinsfile in Git repo keeps CI/CD tools with source

**Build Results:**
- Dashboard provides at-a-glance status
- Console output for detailed debugging
- Test results displayed via JUnit plugin
- Email notifications for failures

---

## Lesson 6: Next Steps

### Learning Objectives

**What are the learning objectives?**
- Identify improvements to move from demo to production-grade CI/CD system
- Understand multi-developer considerations with STM32CubeIDE and Git
- Address IDE project properties/settings file challenges
- Explore layered Git repository architecture for complex projects

**Why are they important in the context of embedded CI/CD?**
- **Production Readiness:** Demo system works but has limitations for real-world use
- **Team Collaboration:** Multiple developers introduce new challenges with IDE-based projects
- **Scalability:** Need strategies for larger teams and more complex projects
- **Security:** Production systems require proper authentication and access control

**How were they implemented?**
- Suggestions presented as ideas (not implemented in course)
- Analysis of potential issues with IDE settings files
- Proposed solutions: standardized paths, layered repositories
- Discussion of network-based repos and GitHub integration

### Pipeline Improvements for Production

### 1. Replace Simple Local Repo with Network-Based Git Server

**Current Demo Setup:**
- File-based repo on laptop
- Fine for demo, not realistic for production

**Option A: Private Git Server**
```
Network-based private Git server for remote repo
- Physical server OR cloud-based
- May run in Docker container
- Secure interface with keys (not passwords)
```

**Impact on Developers/Jenkins:**
- Just change URL for remote repo
- Example URL change:
  - From: `file:///C:/Users/gene/repos/cicd_class_1.git`
  - To: `ssh://git.company.com/repos/cicd_class_1.git`

**Option B: Use GitHub**
```
GitHub for remote repo
- Secure interface with keys
- Challenge: Web hook trigger from GitHub to build server through firewall
- GitHub may have ability to do builds (investigate for embedded)
- Consider: How to do HIL testing with GitHub builds?
```

### 2. Prevent Bad Code from Entering Official Branch

**Current System:**
- Push code to remote repo → triggers build
- Code already in official repo when pipeline runs

**Improved System:**
- Code doesn't go into official branch until pipeline succeeds
- Prevents bad code from ever getting in

**Critical Success Factor (From Experience):**
- Automated tests and pipeline must be VERY RELIABLE
- If not reliable:
  - Frustrating to developers
  - Hurts productivity
  - Submissions rejected due to build issues unrelated to their code
- Only implement if pipeline is rock-solid

### 3. Improve Security

**Current Demo Weaknesses:**

Web Hook Trigger:
- Uses username/password
- Should use key-based authentication

Email Notification:
- Uses username/password
- "Less secure app access" (Google may disallow)
- Should use certificates

**Improvements:**
- Key-based authentication for web hook
- Certificate-based authentication for email
- No passwords in configurations

### Multi-Developer Considerations

**Updated Architecture Diagram:**

```
Git Server (Official Repo)
    ↑ push          ↓ fetch
Developer 1 Repo ←→ Developer 2 Repo
    ↓ (various times)
Jenkins Repo (read-only, doesn't change it)
```

**Multiple Repo Copies:**
- Official: Git server
- Jenkins: Read-only copy, fetched as needed
- Developer 1: Working copy, potentially changing
- Developer 2: Working copy, potentially changing
- Developers push changes to official at various times

### Issue 1: Local Environment Data in Settings Files

**Problem:**
IDE project properties/settings files may contain data specific to developer's local environment.

**Example:**
Settings file contains full path names:
```
# In .mxproject file
project.path=C:\Users\gene\workspace\cicd_class_1
```

**Concern:**
- Developer A has: `C:\Users\alice\workspace\cicd_class_1`
- Developer B has: `C:\Users\bob\my_projects\cicd_class_1`
- Settings files differ even though logically identical

**Simple/Practical Solution:**
```
Fixed location for all developers
Project convention: Everyone must put IDE project folder at same location
Example: All developers use C:\embedded\cicd_class_1

Result: Path names in settings files identical for everyone
```

### Issue 2: Non-Identical Settings Files

**Problem:**
Settings files that are logically identical but not literally identical.

**Example:**
- Same settings
- Different line order (not fixed, somewhat random)
- Different developers have different file representations

**Result: "Whipsawed" Settings Files**
```
1. Developer A pushes changes with their settings file version
2. Developer B pulls, makes unrelated change, pushes
3. Even though settings identical, file looks different from Git's perspective
4. Settings file appears to change in Git history constantly
5. Repo history polluted with meaningless settings file "changes"
```

**Impact:**
- May work, but not ideal
- Confusing Git history
- Hard to track real changes

### Proposed Solution: Layered Repository Architecture

**Concept:**
Layer project files into several repos based on file characteristics.

**Three-Layer Architecture:**

**Layer 1: Application Code (Hand-Written)**
- App source files
- NOT supplied by IDE
- Exactly same for all developers
- May hook into IDE code (main, exception handlers)
- May be common to multiple MCUs (use `#ifdef`)
- Might appear in multiple IDE projects
- Repo: `application_code.git` (or per-developer)

**Layer 2: IDE-Maintained Files (Identical for All)**
- Generated code
- Libraries copied into project
- Makefiles (generated)
- Linker scripts (generated)
- Build files (generated)
- `.ioc` file (hardware config)
- These files NEEDED for CI/CD builds
- Repo: `cicd_build_files.git` (shared)

**Layer 3: Per-Developer Settings (May Differ)**
- Properties/settings files
- NOT required for CI/CD builds
- Only need code + makefiles for builds
- Repo: `developer_settings.git` (per-dev or folders)
- OR: Single repo with per-developer folders

**Key Requirement:**
Each developer must ensure:
- With their IDE properties/settings repo
- If they regenerate code in Layer 2
- Will get same generated code as everyone else

**Implementation Considerations:**
- Require method to layer repos in IDE project folder
- Methods for how to make changes
- How to push changes
- How to handle coordinated changes between layers

### Key Takeaways

**Production System Improvements:**
- Replace local file repo with network-based Git server or GitHub
- Implement security: key-based authentication, not passwords
- Consider preventing bad code from entering official branch (requires very reliable pipeline)

**Multi-Developer Challenges:**
- IDE settings files may contain local environment data (full paths)
- Settings files may be logically identical but literally different (line order)
- Can result in "whipsawed" files in Git history

**Solutions for Multi-Developer:**
- **Simple:** Standardize project locations for all developers (same path)
- **Advanced:** Layered repository architecture
  - Separate concerns: app code, build files, settings
  - Different storage strategies for different layers
  - Only build files needed for CI/CD

**Layered Repo Benefits:**
- Application code can be shared across multiple projects
- Build files guaranteed identical (required for CI/CD)
- Per-developer settings isolated, don't pollute shared history
- Cleaner Git history, easier to track meaningful changes

---

## Course Summary

### What Was Built

A complete embedded CI/CD system with hardware-in-the-loop testing:

1. **Git Repository Setup** - Local and remote repos for source code management
2. **Build Automation** - Command-line scripts for Debug and Release builds
3. **Flash Programming** - Automated flash memory programming via STM32_Programmer_CLI
4. **Static Code Analysis** - cpp-check integration for bug detection
5. **HIL Testing** - Python-based test automation with actual hardware
6. **Jenkins Pipeline** - Automated build server coordinating all steps
7. **Git Web Hooks** - Automatic build triggers on code push
8. **Email Notifications** - Automated alerts on build failures
9. **Test Reporting** - JUnit XML integration for test result visualization

### Critical Lessons Learned

1. **Build ID Verification is Critical** - Ring doorbell story demonstrates importance of verifying correct software version during testing

2. **Makefile Management** - Developer workflow must ensure makefiles stay current by building both Debug and Release before committing

3. **Minimal HIL Testing Has Value** - Even just verifying software boots prevents "dead on arrival" loads

4. **Static Analysis Catches Hidden Bugs** - Finds issues compilers miss (null pointer dereference example)

5. **Pipeline Reliability is Paramount** - For multi-developer teams, unreliable pipelines hurt productivity more than they help

6. **Lean Jenkinsfile Approach** - Keep pipeline scripts simple, delegate actual work to lower-level scripts for greater tool independence

### Technologies Mastered

- **Git:** Command-line operations, bare repos, web hooks
- **Jenkins:** Declarative pipelines, parameterized builds, plugin configuration
- **Python:** Test automation with p-expect, JUnit XML generation
- **Windows Batch:** Build and flash automation scripts
- **STM32 Tools:** STM32CubeIDE, STM32_Programmer_CLI
- **cpp-check:** Static code analysis integration

### Course Philosophy

"Doing just a little is better than doing nothing" - Start with basic automation and build up over time. CI/CD is achievable even with GUI-based IDEs through proper scripting and workflow design.

### GitHub Resources

- Main Nucleo project repository
- Blue Pill HIL simulation hardware project

---

**Course Complete:** 6 lessons covering embedded CI/CD from initial setup through production considerations

**Instructor:** Gene Schrader
**Final Message:** "That is the end of this lesson and the end of the course on embedded CI/CD with hardware in the loop testing. I hope you found it useful and again thanks for watching."

---
---

# Best Practices & Terminologies Guide

## Introduction

This comprehensive guide extracts and organizes all best practices, methodologies, tools, and technical concepts from Gene Schrader's embedded CI/CD course. It's designed as a learning-oriented reference to help you understand production-level coding practices that make code more reliable, maintainable, and testable in CI/CD environments.

The guide is organized into 8 categories, each building on foundational concepts to more advanced practices. Each entry includes:
- Clear definitions
- Why it's important for production code
- How it's used in CI/CD workflows
- Concrete examples from the course
- Related concepts for deeper understanding

**Target Audience:** Developers who know how to code but want to elevate their skills to production-level with industry best practices.

---

## Table of Contents

1. [Source Control & Version Management Best Practices](#category-1-source-control--version-management-best-practices)
2. [Build & Automation Best Practices](#category-2-build--automation-best-practices)
3. [Testing Best Practices](#category-3-testing-best-practices)
4. [CI/CD Pipeline Best Practices](#category-4-cicd-pipeline-best-practices)
5. [Code Quality & Analysis Best Practices](#category-5-code-quality--analysis-best-practices)
6. [Security & Production Readiness Best Practices](#category-6-security--production-readiness-best-practices)
7. [Team Collaboration & Workflow Best Practices](#category-7-team-collaboration--workflow-best-practices)
8. [Tools & Technologies Reference](#category-8-tools--technologies-reference)

---

## Category 1: Source Control & Version Management Best Practices

### Local Repository (Local Repo)

**Definition:**
A Git repository that exists on a developer's machine, containing the working copy of the code that can be directly edited and tested.

**Why It's Important:**
Local repositories allow developers to work independently, make frequent commits, experiment without affecting others, and maintain full version history locally. This is fundamental to distributed version control and enables offline work.

**How It's Used in CI/CD:**
In the course, each developer has a local Git repository in their STM32CubeIDE project folder. They make changes, commit locally to build history, and push to the remote repo when ready. This workflow separates in-progress work from official code.

**Example from Gene's Course:**
```bash
cd C:\Users\gene\workspace\cicd_class_1
git init  # Creates local repo
git add .
git commit -m "IDE create project"
# Local commits build up history before pushing to remote
```

The developer makes three incremental commits locally (IDE create project, add app code, add makefiles) before pushing to remote repo.

**Related Concepts:**
- Remote Repository (official code storage)
- Working Copy/Working Directory (files you edit)
- Staging Area (git add)
- Bare Repository

---

### Remote Repository (Remote Repo)

**Definition:**
A Git repository hosted on a server that serves as the "official" or authoritative copy of the code, shared between multiple developers and build systems.

**Why It's Important:**
Remote repositories enable code sharing, serve as backup, provide single source of truth, and integrate with CI/CD systems. They're the hub that connects all developers and automated systems.

**How It's Used in CI/CD:**
The remote repo receives pushes from developers and triggers Jenkins builds via web hooks. Jenkins fetches code from this repo to build and test. Successfully tested builds are the official versions.

**Example from Gene's Course:**
```bash
# Create bare repo as remote
cd C:\Users\gene\repos
mkdir cicd_class_1.git
cd cicd_class_1.git
git init --bare

# Connect local to remote
cd C:\path\to\ide\project
git remote add origin file:///C:/Users/gene/repos/cicd_class_1.git
git push -u origin master
```

After push, Git hook triggers Jenkins build automatically.

**Related Concepts:**
- Bare Repository (type of remote repo)
- Git Hooks (automation triggers)
- Push/Pull operations
- Clone operation

---

### Bare Repository

**Definition:**
A Git repository that contains only the Git history and metadata (`.git` contents) without a working directory for editing files. Used exclusively for sharing and storage.

**Why It's Important:**
Bare repositories are the standard for central/shared repositories because they prevent direct file editing, which could cause confusion. They serve purely as storage and distribution points, ensuring all changes come through proper Git operations.

**How It's Used in CI/CD:**
The official remote repo is a bare repository. Developers cannot accidentally edit files directly; they must push changes from their local repos. Jenkins fetches code from this bare repo to build.

**Example from Gene's Course:**
```bash
# Creating bare repo
git init --bare  # Note the --bare flag

# Directory structure is different:
# Contains: HEAD, branches/, config, description, hooks/, info/, objects/, refs/
# Does NOT contain: working files or folders like src/, app/, etc.
```

The `.git` suffix in `cicd_class_1.git` is a naming convention signaling it's a bare repository.

**Related Concepts:**
- Remote Repository
- Working Directory (what bare repos lack)
- Git Push/Pull (how to interact with bare repos)
- Clone operation

---

### .gitignore File

**Definition:**
A special file that tells Git which files and patterns to ignore, preventing them from being tracked or appearing in `git status` as untracked files.

**Why It's Important:**
Keeps the repository clean and focused on source code by excluding generated files, build outputs, temporary files, and IDE-specific files. Essential for maintaining a lean, meaningful repository and avoiding accidental commits of binary files or sensitive data.

**How It's Used in CI/CD:**
The `.gitignore` file prevents build output files (`.o`, `.elf`, `.hex`, etc.) from cluttering the repository. Only source files and scripts needed for rebuilding are tracked. This ensures Jenkins can do clean builds from source.

**Example from Gene's Course:**
```gitignore
# Build output files (don't store in repo)
*.o
*.d
*.su
*.map
*.elf
*.hex
*.bin
*.list

# Exception: objects.list is required for build
!objects.list

# Test results (generated during HIL testing)
test_results*.xml
```

The `!objects.list` line uses an exception pattern - learned "the hard way" when builds failed without it.

**Related Concepts:**
- Build Output Files
- Source vs Generated Files
- Git Status command
- Repository Cleanliness

---

### Incremental Commits

**Definition:**
A practice of making small, focused commits that each represent a logical unit of work, rather than large commits containing many unrelated changes.

**Why It's Important:**
Creates meaningful history showing how the project evolved, enables easier rollback of specific changes, facilitates code review, and helps with debugging by allowing bisection to find when issues were introduced.

**How It's Used in CI/CD:**
In the course, the project is built up through incremental commits: first the IDE-generated project, then application code, then makefiles after builds. Each commit represents a complete, logical step that could be individually reviewed or reverted.

**Example from Gene's Course:**
```bash
# Commit 1: Base project
git commit -m "IDE create project"

# Commit 2: Add application
git commit -m "Add app and cicd scripts"

# Commit 3: After building both configs
git commit -m "Debug and release builds"
```

Three separate commits instead of one large commit. Using `git log` shows clear progression. Using `git diff` between commits shows specific changes at each step.

**Related Concepts:**
- Commit Messages (describing changes)
- Git Log (viewing history)
- Git Diff (comparing commits)
- Atomic Commits
- Code Review

---

### Git Hooks

**Definition:**
Scripts that Git automatically executes before or after specific Git events (commit, push, receive, update, etc.). Can be client-side (local repo) or server-side (remote repo).

**Why It's Important:**
Enables automation at critical points in the development workflow. Server-side hooks can enforce policies, trigger builds, send notifications, or perform validation before accepting code.

**How It's Used in CI/CD:**
A post-update hook on the server automatically triggers Jenkins builds whenever code is pushed. This creates the "continuous" part of CI/CD - no manual intervention needed to start builds.

**Example from Gene's Course:**
```bash
# File: cicd_class_1.git/hooks/post-update
#!/bin/bash
curl -u gene:password http://localhost:8080/generic-webhook-trigger/invoke?token=cicd_class_1
```

This server-side hook runs AFTER push completes, sends HTTP request to Jenkins, triggering a build. The token identifies which Jenkins project to run.

**Related Concepts:**
- Web Hooks (triggering external systems)
- CI/CD Automation
- Post-Update Hook (specific type)
- Build Triggers

---

### git status Command

**Definition:**
A Git command that shows the state of the working directory and staging area - which files are modified, staged for commit, or untracked.

**Why It's Important:**
Essential for understanding what changes exist and what will be included in the next commit. Helps verify correct files were modified and catches accidental changes. The `-u` flag shows untracked files in detail.

**How It's Used in CI/CD:**
Before committing, developers use `git status -u` to verify only intended files changed and no unexpected generated files appear (which should be in `.gitignore`). This prevents committing the wrong files.

**Example from Gene's Course:**
```bash
git status -u
# Shows:
# - Modified: Core/Src/main.c, .cproject
# - Untracked: app/ folder, cicd_tools/ folder
# - Helps verify .gitignore working (no .o, .elf files shown)

git add .
git commit -m "Add app and cicd scripts"
```

The `-u` flag is crucial - without it, you might miss seeing untracked files that should be committed.

**Related Concepts:**
- Working Directory
- Staging Area
- Untracked Files
- .gitignore file
- git add command

---

### git diff Command

**Definition:**
A Git command that shows differences between commits, branches, working directory and staging area, or any two points in Git history.

**Why It's Important:**
Allows reviewing changes before committing, understanding what changed between versions, and investigating when bugs were introduced. Essential for code review and debugging.

**How It's Used in CI/CD:**
Developers use `git diff` to review changes before committing to ensure only intended modifications are included. Historical diffs show evolution of code and help understand impact of changes.

**Example from Gene's Course:**
```bash
# See changes between commits
git diff e6234cb 107d715 Core/Src/main.c
# Shows the app_main() call was added to main.c

# Just see which files changed
git diff e6234cb 107d715 --name-only
# Output: Core/Src/main.c, app/gpioapp/appmain.c, cicd_tools/build.bat, ...

# See current uncommitted changes
git diff
```

**Related Concepts:**
- Git Log (finding commit hashes)
- Code Review
- Commit History
- Staging Area

---

### git log Command

**Definition:**
A Git command that displays the commit history, showing commit hashes, authors, dates, and commit messages in reverse chronological order.

**Why It's Important:**
Provides visibility into project evolution, helps identify when changes were made and by whom, enables finding specific commits for diff or revert operations, and serves as project documentation.

**How It's Used in CI/CD:**
Developers use `git log` to find commit hashes for diffing, understand recent changes, and track project progression. CI/CD systems use commit information for build identification and change tracking.

**Example from Gene's Course:**
```bash
git log
# Output shows three commits:
# commit 107d715... "Debug and release builds"
# commit e6234cb... "Add app and cicd scripts"
# commit abc1234... "IDE create project"

# Use hashes with git diff to compare
git diff e6234cb 107d715 --name-only
```

**Related Concepts:**
- Commit Hash (unique identifier)
- Git Diff (comparing commits)
- Commit Messages
- Project History

---

### Push, Pull, Fetch Operations

**Definition:**
Git commands for synchronizing between local and remote repositories:
- **Push**: Sends local commits to remote repo
- **Pull**: Fetches remote changes and merges into local branch
- **Fetch**: Downloads remote changes without merging

**Why It's Important:**
These operations enable distributed collaboration. Push makes your work available to others and triggers CI/CD. Pull keeps you synchronized with team changes. Fetch allows inspecting remote changes before integrating.

**How It's Used in CI/CD:**
Developers push to trigger builds. Jenkins fetches to get latest code. In multi-developer scenarios, developers pull to get teammates' changes before pushing their own work.

**Example from Gene's Course:**
```bash
# Initial push with upstream tracking
git push -u origin master  # -u sets default remote/branch

# Subsequent pushes (triggers Jenkins via hook)
git push  # Simplified after -u

# Jenkins workspace (conceptually)
git fetch origin master  # Gets latest code
# Build, test, analyze...
```

**Related Concepts:**
- Remote Repository
- Git Hooks (triggered by push)
- Merge Conflicts
- Branch Tracking

---

### Branches and Main/Master Branch

**Definition:**
Branches are parallel lines of development in Git. The main/master branch is conventionally the primary, stable branch representing production-ready code.

**Why It's Important:**
Branches enable parallel development without interference, support feature isolation, facilitate code review through pull requests, and provide safety by keeping experimental work separate from stable code.

**How It's Used in CI/CD:**
The course uses the master branch as the primary development branch. Pushes to master trigger CI/CD builds. Production systems often protect the main branch and require pull requests with passing tests before merging.

**Example from Gene's Course:**
```bash
# Work happens on master branch
git push -u origin master

# Jenkinsfile configured to build from master:
# Branch: master
# Pushes to master trigger post-update hook
```

Lesson 6 discusses protecting main branch - code doesn't enter until pipeline succeeds (requires reliable tests).

**Related Concepts:**
- Feature Branches (not used in course, but common practice)
- Pull Requests
- Branch Protection
- Git Merge

---

### Clone Operation

**Definition:**
A Git command that creates a complete copy of a remote repository, including all history, branches, and files, establishing a new local repository.

**Why It's Important:**
Cloning is how new team members or build systems get their initial copy of the codebase. It establishes the link between local and remote repos and sets up tracking relationships.

**How It's Used in CI/CD:**
Jenkins performs an initial clone (or uses workspace management) to get the project code. New developers clone to start working. Each clone is independent but connected to the remote for synchronization.

**Example from Gene's Course:**
```bash
# Jenkins workspace initialization (conceptual)
git clone file:///C:/Users/gene/repos/cicd_class_1.git

# Or with specific branch
git clone -b master file:///C:/Users/gene/repos/cicd_class_1.git
```

Jenkins then fetches updates on subsequent builds rather than re-cloning each time (more efficient).

**Related Concepts:**
- Remote Repository
- Working Directory
- Git Fetch (updating after clone)
- Repository URL

---

## Category 2: Build & Automation Best Practices

### Make and Makefiles

**Definition:**
Make is a build automation tool that uses Makefiles to define how to compile and link programs. Makefiles contain rules specifying dependencies between files and commands to build targets.

**Why It's Important:**
Make provides declarative build specification, automatic dependency tracking, incremental builds (only recompiling changed files), and platform-independent build descriptions. It's industry standard for C/C++ projects.

**How It's Used in CI/CD:**
STM32CubeIDE generates Makefiles for Debug and Release configurations. The build automation scripts invoke `make` directly rather than using the IDE, providing independence from the GUI and enabling Jenkins to build without the IDE running.

**Example from Gene's Course:**
```bash
# IDE generates makefiles in Debug/ and Release/ folders
# Build script invokes make:
cd Debug
make -j4 all  # -j4 = parallel build with 4 jobs

# Makefile contains (simplified):
# all: cicd_class_1.elf
# cicd_class_1.elf: $(OBJS)
#     $(CC) $(LDFLAGS) -o $@ $^ -T$(LDSCRIPT)
# %.o: %.c
#     $(CC) $(CFLAGS) -c $< -o $@
```

**Related Concepts:**
- Build Targets (all, clean)
- Make Targets
- Incremental Builds
- Compiler/Linker
- Build Dependencies

---

### Make Targets

**Definition:**
Named goals in a Makefile that specify what to build or what action to perform. Common targets include `all` (build everything), `clean` (remove build outputs), and specific file targets.

**Why It's Important:**
Targets provide flexible build control - you can build specific components, clean before rebuilding, or run tests. They're the interface for controlling the build process from scripts or command line.

**How It's Used in CI/CD:**
Build scripts use different targets for different operations: `clean` removes old build outputs, `all` builds the complete project. Jenkins scripts call both for clean builds.

**Example from Gene's Course:**
```batch
REM Build script usage
build.bat Debug clean    REM Removes .o, .elf files
build.bat Debug all      REM Compiles and links everything

REM Translates to make commands:
make clean  # Removes build outputs
make all    # Builds cicd_class_1.elf
```

**Related Concepts:**
- Makefile
- Build Automation
- Clean Build vs Incremental Build
- Build Scripts

---

### Build Automation

**Definition:**
The practice of scripting the entire build process so it can be executed with a single command, without manual intervention or IDE interaction.

**Why It's Important:**
Automation ensures consistency (same build process every time), enables CI/CD integration, reduces human error, documents the build process, and allows builds on servers without GUIs.

**How It's Used in CI/CD:**
Windows batch scripts automate building by setting environment variables and invoking make. Jenkins calls these scripts to build without human interaction. The scripts handle both Debug and Release configurations.

**Example from Gene's Course:**
```batch
@echo off
setlocal

REM Build script: build.bat
set CWD=C:\Users\gene\workspace\cicd_class_1\Debug
set PATH=C:\ST\toolchain\bin;%PATH%

cd /d %CWD%
make -j4 all

REM Production script parameterized:
REM build.bat <config> <target> <project_path> <toolchain_path> <build_id>
```

**Related Concepts:**
- Build Scripts
- Headless Builds
- Make/Makefiles
- Environment Variables
- Repeatability

---

### Headless IDE Build

**Definition:**
Running an IDE in "headless" mode means executing it via command line without displaying the GUI. The IDE performs work (compilation, linking) without user interface.

**Why It's Important:**
Allows using IDE's build capabilities in automated environments like CI/CD servers. However, adds dependency on IDE installation and can have reliability issues.

**How It's Used in CI/CD:**
The course CONSIDERED this approach but REJECTED it in favor of make-based builds due to instructor's doubts about reliability and desire for IDE independence.

**Example from Gene's Course:**
```bash
# Headless approach (NOT USED):
STM32CubeIDE --headless --build path/to/project

# Instead, used make directly:
make -j4 all  # More reliable, no IDE dependency
```

**Instructor's Experience:** "Had issues, real doubts about reliability. Did NOT use this option."

**Related Concepts:**
- Build Automation
- Make-based Builds (chosen alternative)
- IDE Independence
- Tool Chain

---

### Build Scripts (Batch Scripts)

**Definition:**
Scripts (Windows .bat files in this course) that automate the build process by setting up the environment and invoking build tools with appropriate parameters.

**Why It's Important:**
Encapsulate build knowledge, provide consistent interface, handle complexity of paths and environment variables, and enable both manual and automated builds from the same scripts.

**How It's Used in CI/CD:**
Two sets of scripts exist: simplified scripts for IDE developers (`build_i.bat`) and production scripts for Jenkins with parameterized inputs. Both invoke make but handle environment differently.

**Example from Gene's Course:**
```batch
REM Simple IDE version: build_i.bat
@echo off
set CWD=C:\Users\gene\workspace\cicd_class_1\%1
cd /d %CWD%
make -j4 %2

REM Usage:
build_i.bat Debug all
build_i.bat Release clean

REM Production version (called by Jenkins):
build.bat Debug all %WORKSPACE% %TOOLCHAIN_PATH% %BUILD_ID%
```

**Related Concepts:**
- Build Automation
- Environment Variables
- Make/Makefiles
- Parameterization
- Scripting

---

### Build ID / Build Version

**Definition:**
A unique identifier assigned to each build, typically including build number, date/time, or commit hash. Embedded into the software at build time.

**Why It's Important:**
CRITICAL for testing verification - ensures you're testing the correct software version. Prevents disasters like testing old code while believing it's new. Enables traceability between code changes and test results.

**How It's Used in CI/CD:**
Jenkins provides BUILD_ID, which build script writes into `version.h`. Application code includes this header and provides "version" command. Test script verifies version matches expected BUILD_ID before testing.

**Example from Gene's Course:**
```batch
REM build.bat creates version.h
echo #ifndef VERSION_H > version.h
echo #define VERSION_H >> version.h
echo #define BUILD_VERSION "%BUILD_ID%" >> version.h
echo #endif >> version.h

REM Application can print:
printf("version=%s\n", BUILD_VERSION);

REM Test script verifies:
python base_hilt.py --tver jenkins-cicd_class_1-42
# Script sends "version" command
# Expects "version=jenkins-cicd_class_1-42"
# Fails test if mismatch
```

**Story:** Ring doorbell shipped untested software due to testing wrong version - "gut-wrenching" discovery.

**Related Concepts:**
- Build Verification
- Test Automation
- Version Control
- Ring Doorbell Story (lesson learned)

---

### Debug vs Release Builds

**Definition:**
Two build configurations with different optimization and debugging settings:
- **Debug**: Includes debug symbols, no optimization, enables debugging
- **Release**: Optimized for size/speed, minimal debug info, production-ready

**Why It's Important:**
Debug builds enable effective development and debugging. Release builds produce efficient code for deployment. Both must be tested because optimization can expose bugs not present in debug builds.

**How It's Used in CI/CD:**
The course maintains separate makefiles for Debug and Release. Developers build both before committing to ensure makefiles stay current. Jenkins builds and tests BOTH configurations in the pipeline.

**Example from Gene's Course:**
```bash
# Developer workflow before committing:
# 1. Build Debug in IDE
# 2. Build Release in IDE (important!)
# 3. Commit (includes updated makefiles for both)

# Jenkins pipeline:
bat "build.bat Debug all ..."      # Build debug
bat "build.bat Release all ..."    # Build release
# Flash and test debug
# Flash and test release
```

**Common Issue:** Only building Debug before committing → Release makefiles outdated → Jenkins Release build fails.

**Related Concepts:**
- Build Configurations
- Compiler Optimization
- Makefile Management
- Testing

---

### Clean Build vs Incremental Build

**Definition:**
- **Clean Build**: Removes all previous build outputs and rebuilds everything from scratch
- **Incremental Build**: Only recompiles files changed since last build, reuses existing object files

**Why It's Important:**
Clean builds ensure no artifacts from previous builds interfere (important for release builds and troubleshooting). Incremental builds save time during development. CI/CD often uses clean builds for reliability.

**How It's Used in CI/CD:**
Jenkins typically does clean builds to ensure reproducibility. Developers use incremental builds during development for speed, but do clean builds before important commits or when troubleshooting build issues.

**Example from Gene's Course:**
```batch
REM Clean then build:
build_i.bat Debug clean  # Remove old outputs (fast)
build_i.bat Debug all    # Build everything (slower)

REM Incremental:
build_i.bat Debug all    # Only recompiles changed files
```

Make automatically determines what needs rebuilding based on file timestamps and dependencies.

**Related Concepts:**
- Make Targets (clean, all)
- Build Dependencies
- Build Time Optimization
- Reproducible Builds

---

### Tool Chain (Compiler/Linker)

**Definition:**
A collection of programming tools used to build software, primarily the compiler (translates source to object code) and linker (combines object files into executable), plus associated utilities.

**Why It's Important:**
The tool chain determines which microcontroller you can target, what language features are available, and how code is optimized. Consistent tool chain usage across team and CI/CD is essential for reproducible builds.

**How It's Used in CI/CD:**
STM32CubeIDE includes an ARM GCC tool chain. Build scripts set PATH to include tool chain binaries. Both developers and Jenkins use the same tool chain version to ensure consistent builds.

**Example from Gene's Course:**
```batch
REM Build script sets tool chain path:
set PATH=C:\ST\STM32CubeIDE\plugins\...\tools\bin;%PATH%

REM Now make can find:
# arm-none-eabi-gcc (compiler)
# arm-none-eabi-ld (linker)
# arm-none-eabi-objcopy (creates .hex, .bin)

REM Makefile uses:
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc  # Often gcc wraps ld
```

**Related Concepts:**
- Compiler
- Linker
- Cross-Compilation (ARM target, x86 host)
- Build Environment

---

### Environment Variables

**Definition:**
System variables that store configuration information and are accessible to all processes. Used to pass information to build tools without hard-coding paths in scripts.

**Why It's Important:**
Enable flexible, portable scripts that work across different systems. Allow parameterization of builds without editing multiple files. Essential for CI/CD where paths differ between developer machines and build servers.

**How It's Used in CI/CD:**
Build scripts extract CWD and PATH from IDE properties, set them as environment variables, then invoke make. Jenkins provides WORKSPACE, BUILD_ID, and other variables used by scripts.

**Example from Gene's Course:**
```batch
REM IDE provides via Project Properties:
set CWD=C:\Users\gene\workspace\cicd_class_1\Debug
set PATH=C:\ST\...\bin;%PATH%

REM Jenkins provides:
# WORKSPACE=/var/jenkins/workspace/cicd_class_1
# BUILD_ID=jenkins-cicd_class_1-42
# COM_DUT=COM13 (parameterized)

REM Scripts use:
cd /d %CWD%
echo #define BUILD_VERSION "%BUILD_ID%" >> version.h
```

**Related Concepts:**
- Build Scripts
- Parameterization
- Jenkins Variables
- Build Portability

---

## Category 3: Testing Best Practices

### Hardware-in-the-Loop (HIL) Testing

**Definition:**
A testing methodology where real embedded hardware is integrated into the test setup. Software runs on actual microcontroller/board while test scripts interact with it, simulating external systems and measuring responses.

**Why It's Important:**
Tests actual hardware behavior (not simulation), catches hardware-specific bugs, validates timing and real-world constraints, and provides confidence that software will work in deployed systems. Even minimal HIL testing prevents "dead on arrival" loads.

**How It's Used in CI/CD:**
Two boards are used: Nucleo (device under test) and Blue Pill (simulates external hardware). Python test script controls both via serial connections, sending commands and verifying responses. Tests run automatically in Jenkins pipeline after flashing.

**Example from Gene's Course:**
```python
# Test setup:
# DUT (Nucleo) <--GPIO--> SIM (Blue Pill)
#      |                      |
#   Serial                 Serial
#      |                      |
#  Test Script (Python + p-expect)

# Test sequence:
test_steps = [
    {'device': 'dut', 'command': 'write B 9 1', 'expect': 'OK'},
    {'device': 'sim', 'command': 'read B 9', 'expect': '1'}  # Verify DUT output
]
```

**Minimum valuable test:** Just load software and verify it boots (check for console prompt). Prevents embarrassing "last 5 loads won't boot" situations.

**Related Concepts:**
- Device Under Test (DUT)
- Simulation Hardware
- Integration Testing
- Stimulus-Response Testing

---

### Device Under Test (DUT)

**Definition:**
The hardware/software system being tested - in this case, the product hardware (Nucleo board) running the application code that will eventually be deployed.

**Why It's Important:**
Distinguishes the target system from test equipment and simulation hardware. Clear terminology prevents confusion during test development and troubleshooting.

**How It's Used in CI/CD:**
The Nucleo F401RE board is the DUT. Test scripts send commands to DUT via serial port, control DUT GPIO pins, verify DUT responses, and test that DUT software boots correctly with correct version.

**Example from Gene's Course:**
```python
# Test script configuration:
COM_DUT = 'COM13'  # Serial port for Nucleo board
dut = p-expect.spawn(p-link + COM_DUT)

# Send version command to DUT:
dut.sendline('version')
dut.expect(f'version={target_version}')

# Configure DUT GPIO:
dut.sendline('config B 9 1')  # Set pin as output
dut.sendline('write B 9 1')   # Write high
```

**Related Concepts:**
- HIL Testing
- Simulation Hardware
- Test Automation
- Product Hardware

---

### Simulation Hardware (SIM)

**Definition:**
Hardware used in testing to simulate external systems that the DUT will interact with in real deployment. Acts as test equipment that creates realistic operating conditions.

**Why It's Important:**
Enables realistic testing without needing actual external systems (which might be expensive, dangerous, or unavailable). Provides controllable, repeatable test conditions. Simulates edge cases difficult to create with real systems.

**How It's Used in CI/CD:**
The Blue Pill board simulates external hardware by driving GPIO signals to the DUT and responding to DUT outputs. Same software runs on both boards. Test script controls both to create stimulus-response scenarios.

**Example from Gene's Course:**
```python
# Blue Pill setup:
COM_SIM = 'COM15'
sim = p-expect.spawn(p-link + COM_SIM)

# Test: DUT writes, SIM reads
sim.sendline('config B 9 0')  # SIM pin as input
# DUT writes 1
sim.sendline('read B 9')
sim.expect('1')  # Verify SIM received DUT output
```

Same GPIO application software runs on both boards - simple approach that works well for demonstration.

**Related Concepts:**
- HIL Testing
- Device Under Test
- Test Fixtures
- GPIO Testing

---

### Unit Tests

**Definition:**
Tests that verify individual functions, methods, or modules in isolation, typically using test stubs/mocks for dependencies. Focus on small, testable units of code.

**Why It's Important:**
Catch bugs early at the function level, enable test-driven development, provide fast feedback (no hardware needed), document expected behavior, and make refactoring safer.

**How It's Used in CI/CD:**
The course mentions unit tests as one type of automated testing but doesn't implement them. Unit tests typically run on the build host, not embedded target, and are faster than HIL tests but don't verify hardware behavior.

**Example from Gene's Course:**
```c
// Unit test example (conceptual, not in course):
// Test get_command_line() function
void test_parse_command() {
    char* tokens[10];
    int count = parse_command("write B 9 1", tokens);
    assert(count == 4);
    assert(strcmp(tokens[0], "write") == 0);
    assert(strcmp(tokens[1], "B") == 0);
}
```

Course emphasizes HIL testing instead, noting "even small amount of HIL testing is valuable."

**Related Concepts:**
- Integration Tests
- Test Stubs/Mocks
- HIL Testing (different approach)
- Test-Driven Development

---

### Integration Tests

**Definition:**
Tests that verify how multiple components work together. Higher-level than unit tests but may still use simulated/mocked hardware rather than actual hardware.

**Why It's Important:**
Catch bugs in component interactions, verify interfaces between modules, test more realistic scenarios than unit tests, but still faster and more controllable than full system tests.

**How It's Used in CI/CD:**
Course mentions integration tests as intermediate between unit tests and HIL tests. May run on build host with hardware simulation. See `#ifdef` usage for special test builds.

**Example from Gene's Course:**
```c
// Integration test characteristics mentioned:
// - Run on build host (not embedded target)
// - Use test stubs/harnesses (additional software)
// - Product code may need special builds:

#ifdef INTEGRATION_TEST
    // Use mock UART for testing
    mock_uart_init();
#else
    // Use real HAL UART
    HAL_UART_Init(&huart2);
#endif
```

Course philosophy: "More types of testing = better. Decision is where to spend time."

**Related Concepts:**
- Unit Tests
- HIL Testing
- Test Harnesses
- Mocking/Stubbing

---

### Test Automation

**Definition:**
The practice of creating automated test scripts that can execute tests without human interaction, verify results automatically, and report pass/fail status programmatically.

**Why It's Important:**
Enables continuous testing in CI/CD, ensures tests run consistently every time, eliminates human error in test execution, provides fast feedback, and makes frequent testing practical.

**How It's Used in CI/CD:**
Python scripts automate HIL testing by sending serial commands, verifying responses using p-expect, and generating JUnit XML reports. Jenkins runs these scripts automatically after each build.

**Example from Gene's Course:**
```python
# base_hilt.py - Automated test script
def test_gpio_output():
    # Send commands
    dut.sendline('config B 9 1')
    dut.expect('OK', timeout=2)
    dut.sendline('write B 9 1')
    dut.expect('OK', timeout=2)

    # Verify on simulation hardware
    sim.sendline('read B 9')
    sim.expect('1', timeout=2)

    # Return pass/fail
    return True

# Run from Jenkins:
bat "python3 base_hilt.py --tver ${BUILD_ID} --com_dut ${COM_DUT}"
```

**Related Concepts:**
- Test Scripts
- HIL Testing
- CI/CD Pipeline
- JUnit XML

---

### Stimulus-Response Testing

**Definition:**
A testing approach where the test system provides specific inputs (stimulus) to the DUT and verifies the outputs (response) match expected behavior. Also called black-box testing when internal state isn't observed.

**Why It's Important:**
Validates external behavior (what users/systems see), doesn't require knowledge of internal implementation, catches integration issues, and mirrors real-world usage patterns.

**How It's Used in CI/CD:**
Test script sends commands via serial port (stimulus) and verifies responses using p-expect pattern matching. GPIO tests write to one board and read from another to verify electrical signals.

**Example from Gene's Course:**
```python
# Stimulus-response pattern:
# Stimulus: Send command
dut.sendline('write B 9 1')

# Response: Expect confirmation
dut.expect('OK', timeout=2)

# Stimulus: Query sim hardware
sim.sendline('read B 9')

# Response: Verify value
sim.expect('1', timeout=2)

# If response doesn't match, test fails
```

Based on "expect" tool from 1990s - time-tested approach for serial communication testing.

**Related Concepts:**
- HIL Testing
- p-expect Tool
- Black-box Testing
- Serial Communication

---

### Data-Driven Tests

**Definition:**
A test design approach where test logic is separated from test data. Test data is stored in data structures (arrays, dicts, JSON, etc.) and a generic test engine iterates through the data to execute tests.

**Why It's Important:**
Reduces code duplication, makes adding new test cases easy (just add data), improves maintainability, makes test intent clearer, and enables non-programmers to contribute test cases.

**How It's Used in CI/CD:**
GPIO tests are defined as arrays of dictionaries specifying device, command, and expected response. A generic test function iterates through steps, sending commands and verifying responses.

**Example from Gene's Course:**
```python
# Data structure defines test:
test_steps = [
    {'device': 'dut', 'command': 'config B 9 1', 'expect': 'OK'},
    {'device': 'sim', 'command': 'config B 9 0', 'expect': 'OK'},
    {'device': 'dut', 'command': 'write B 9 1', 'expect': 'OK'},
    {'device': 'sim', 'command': 'read B 9', 'expect': '1'},
    {'device': 'dut', 'command': 'write B 9 0', 'expect': 'OK'},
    {'device': 'sim', 'command': 'read B 9', 'expect': '0'}
]

# Generic engine executes:
for step in test_steps:
    device = devices[step['device']]
    device.sendline(step['command'])
    device.expect(step['expect'], timeout=2)
```

Instructor: "When lots of repetition, create data structures describing tests."

**Related Concepts:**
- Test Automation
- Test Maintainability
- Separation of Concerns
- Test Data Management

---

### JUnit XML Format

**Definition:**
A standardized XML format for test result reporting, originally from JUnit (Java testing framework) but now widely supported. Contains test counts, pass/fail status, timing, and error messages.

**Why It's Important:**
Provides language-agnostic test result format, enables CI/CD tools to display test results uniformly, allows test trend analysis, and is the de facto standard for test reporting in DevOps.

**How It's Used in CI/CD:**
Python test script uses junit_xml module to generate XML files after testing. Jenkins junit step reads these files and displays results in GUI with trends, statistics, and drill-down to failures.

**Example from Gene's Course:**
```python
# Python generates JUnit XML:
from junit_xml import TestSuite, TestCase

test_cases = [TestCase('test_console_prompt', classname='HIL')]
# ... add more test cases ...

ts = TestSuite("HIL Test Suite", test_cases)
with open('test_results_dut_debug.xml', 'w') as f:
    TestSuite.to_file(f, [ts])

# Jenkinsfile reads results:
junit 'test_results*.xml'

# Jenkins displays:
# "19 tests, 0 disabled, 0 errors, 0 failures"
# Or: "19 tests, 5 failures" with details
```

**Related Concepts:**
- Test Reporting
- CI/CD Integration
- Test Results Visualization
- junit_xml Python Module

---

### Build ID Verification

**Definition:**
A critical testing practice where automated tests verify the software version being tested matches the expected build identifier before executing tests.

**Why It's Important:**
Prevents catastrophic errors where you think you're testing new code but are actually testing old code. Ensures test results are meaningful and associated with correct software version. CRITICAL lesson from Ring doorbell story.

**How It's Used in CI/CD:**
Build script embeds BUILD_ID in version.h. Application includes version command. Test script receives expected version as parameter, queries actual version, and fails immediately if mismatch.

**Example from Gene's Course:**
```python
# Test script parameter:
python base_hilt.py --tver jenkins-cicd_class_1-42

# Version verification test (ALWAYS FIRST):
def test_version(target_version):
    dut.sendline('version')
    dut.expect(f'version={target_version}', timeout=2)
    # If mismatch: FAIL test immediately
    # Don't proceed with other tests

# Application code:
printf("version=%s\n", BUILD_VERSION);  // From version.h
```

**Ring Story:** Shipped doorbell with untested software because test setup had wrong version. Video quality problems. "Gut-wrenching" discovery. Lucky cloud-side fix possible.

**Related Concepts:**
- Build ID/Version
- Test Reliability
- Ring Doorbell Story
- Version Control

---

### Test Results and Test Failures

**Definition:**
Structured output from automated tests indicating which tests passed, failed, or had errors, along with timing, error messages, and other diagnostic information.

**Why It's Important:**
Provide immediate feedback on code quality, identify regressions quickly, guide debugging efforts, support quality metrics and trends, and determine if builds should be promoted or rejected.

**How It's Used in CI/CD:**
JUnit XML files contain test results. Jenkins reads them for display. check_test_results.bat script examines XML and fails the build stage if any test failures exist, preventing bad builds from progressing.

**Example from Gene's Course:**
```python
# Successful test run:
# Output: "19 tests, 0 disabled, 0 errors, 0 failures"

# Failed test run (wrong version + removed jumper):
# Output: "19 tests, 5 failures"
# - Version test failed (pattern not matched)
# - 4 GPIO tests failed (no electrical connection)

# Jenkinsfile:
junit 'test_results*.xml'  # Display in GUI
bat "check_test_results.bat test_results*.xml"  # Fail stage if failures
```

**Pipeline Behavior:** Test failures cause build to fail, preventing delivery and triggering email notification.

**Related Concepts:**
- JUnit XML Format
- Test Automation
- CI/CD Pipeline
- Quality Gates

---

### Black-box vs White-box Testing

**Definition:**
- **Black-box**: Testing based only on external behavior/interfaces without knowledge of internal implementation
- **White-box**: Testing that considers internal structure, code paths, and implementation details

**Why It's Important:**
Black-box tests are more robust to implementation changes and test user-visible behavior. White-box tests can achieve better code coverage and test edge cases. Combination is often ideal.

**How It's Used in CI/CD:**
Course HIL testing is primarily black-box - tests send serial commands and verify responses without knowing internal code structure. Instructor notes "Doesn't have to be 100% black-box to be useful."

**Example from Gene's Course:**
```python
# Black-box approach (used):
# Test only knows external interface:
dut.sendline('write B 9 1')  # Don't care how GPIO is implemented
dut.expect('OK')              # Only verify external response

# White-box approach (not used):
# Would require:
# - Knowing internal state variables
# - Setting up specific code paths
# - Accessing internal functions directly
```

**Related Concepts:**
- Stimulus-Response Testing
- Integration Testing
- Test Design
- Code Coverage

---

### Dead-on-Arrival Loads

**Definition:**
Software releases that fail to even boot or start up, rendering the device completely non-functional. Represents the most basic and embarrassing type of software failure.

**Why It's Important:**
Damages credibility severely - if you can't even boot the software, all other quality measures are meaningless. Even minimal testing should catch this. Prevents especially embarrassing situations like "last 5 loads won't boot."

**How It's Used in CI/CD:**
The minimum valuable HIL test is checking that software boots and responds to console prompts. This simple test prevents dead-on-arrival loads from being released.

**Example from Gene's Course:**
```python
# Minimum valuable HIL test:
def test_console_prompt():
    # Just verify software responds
    dut.sendline('')  # Send empty line
    dut.expect('>', timeout=2)  # Check for prompt
    # If no prompt, software didn't boot properly

# This simple test catches:
# - Flash programming failures
# - Initialization crashes
# - Clock configuration problems
# - Basic hardware issues
```

Instructor: "Prevents 'dead on arrival' loads - embarrassing when last 5 loads won't even boot."

**Related Concepts:**
- HIL Testing
- Smoke Testing
- Minimum Viable Testing
- Boot Verification

---

## Category 4: CI/CD Pipeline Best Practices

### Continuous Integration (CI)

**Definition:**
A software development practice where developers frequently integrate code into a shared repository (often daily), with each integration automatically verified by building the project and running automated tests.

**Why It's Important:**
Detects integration problems early when they're easier to fix, provides rapid feedback on code quality, reduces integration complexity, encourages small incremental changes, and keeps the codebase in a working state.

**How It's Used in CI/CD:**
Developers push code to Git remote repo. Web hook automatically triggers Jenkins build. Jenkins builds code, runs static analysis, flashes hardware, and runs HIL tests. Results provided within minutes of push.

**Example from Gene's Course:**
```bash
# Developer workflow:
git add .
git commit -m "Add new feature"
git push  # Triggers CI automatically

# CI pipeline executes:
# 1. Checkout code
# 2. Build Debug and Release
# 3. Static analysis (cpp-check)
# 4. Flash Debug, test Debug
# 5. Flash Release, test Release
# 6. Report results (email if failure)

# Results visible in Jenkins dashboard within minutes
```

**Course Philosophy:** "Well-known and valuable: automation, early feedback, prevents mistakes."

**Related Concepts:**
- Continuous Delivery
- Automated Testing
- Jenkins Pipeline
- Git Hooks

---

### Continuous Delivery vs Continuous Deployment

**Definition:**
- **Continuous Delivery**: Automated process that produces release-ready software but requires manual approval/trigger to deploy to production
- **Continuous Deployment**: Fully automated - successful builds automatically deploy to production without human intervention

**Why It's Important:**
Distinction matters for risk management and compliance. Continuous Delivery provides safety valve (human approval) while maintaining most automation benefits. Continuous Deployment requires extremely high confidence in automated testing.

**How It's Used in CI/CD:**
Course uses Continuous Delivery approach: automated process generates new releases and copies them to software release store when tests pass. Made available to testers/users, but not automatically deployed to end systems.

**Example from Gene's Course:**
```groovy
// Jenkinsfile post-build:
post {
    success {
        bat "deliver.bat"  // Copy images to release store
        // Images now available for deployment
        // But NOT automatically deployed to customer devices
    }
}
```

Release store directory contains tested, ready-to-deploy images. Deployment to actual product devices would be separate manual or controlled step.

**Related Concepts:**
- CI/CD Pipeline
- Release Management
- Deployment Automation
- Quality Gates

---

### Pipeline and Pipeline Stages

**Definition:**
A pipeline is an automated workflow that defines the steps to build, test, and deliver software. Stages are the major phases within a pipeline (e.g., Build, Test, Deploy), each containing one or more steps.

**Why It's Important:**
Pipelines make the entire software delivery process visible, repeatable, and auditable. Stages organize complex workflows into logical phases and provide clear visualization of where builds succeed or fail.

**How It's Used in CI/CD:**
Jenkinsfile defines a declarative pipeline with stages: Checkout, Build, Static Analysis, Flash Debug, Test Debug, Flash Release, Test Release, Post-Build. Each stage has specific purpose and failure in a stage stops progression.

**Example from Gene's Course:**
```groovy
pipeline {
    stages {
        stage('Build') {
            steps {
                bat "build.bat Debug all ..."
                bat "build.bat Release all ..."
            }
        }
        stage('Static Analysis') {
            steps {
                catchError(...) {
                    bat "static_analysis.bat"
                }
            }
        }
        stage('Test Debug') {
            steps {
                bat "python base_hilt.py ..."
                junit 'test_results*.xml'
            }
        }
    }
}
```

**Dashboard:** Shows grid with stages as columns, builds as rows, color-coded results.

**Related Concepts:**
- Declarative Pipeline
- Jenkins
- Stages and Steps
- Build Visualization

---

### Declarative Pipeline vs Scripted Pipeline

**Definition:**
Two styles of writing Jenkins pipelines:
- **Declarative**: Simpler, structured syntax with predefined sections (pipeline, stages, steps). First keyword: `pipeline`
- **Scripted**: More flexible, uses full Groovy programming language. First keyword: `node`

**Why It's Important:**
Declarative pipelines are easier to learn, more readable, and sufficient for most use cases. Scripted pipelines offer more power but higher complexity. Choice affects maintainability and learning curve.

**How It's Used in CI/CD:**
Course uses declarative pipeline for simplicity and popularity. Provides enough flexibility for embedded CI/CD needs while remaining accessible to those new to Jenkins.

**Example from Gene's Course:**
```groovy
// Declarative (USED):
pipeline {
    agent any
    environment { ... }
    stages { ... }
    post { ... }
}

// Scripted (NOT USED):
node {
    stage('Build') {
        // Full Groovy programming
        if (condition) { ... }
        for (config in configs) { ... }
    }
}
```

Instructor: "Declarative pipelines: simpler syntax, more popular. Can contain pieces of advanced script language if needed."

**Related Concepts:**
- Jenkinsfile
- Pipeline Definition
- Groovy Language
- Jenkins Best Practices

---

### Jenkinsfile

**Definition:**
A text file named "Jenkinsfile" that contains the pipeline definition (stages, steps, and configuration) and is typically stored in source control alongside the code.

**Why It's Important:**
Storing pipeline in source control provides version history, enables coordinated changes (code and pipeline together), supports code review of pipeline changes, and ensures pipeline can be recreated if Jenkins fails.

**How It's Used in CI/CD:**
Jenkinsfile lives in the root of the IDE workspace, committed to Git repo. Jenkins configured to fetch Jenkinsfile from Git (SCM) rather than storing it in Jenkins. Pipeline and code evolve together.

**Example from Gene's Course:**
```groovy
// File location: <project_root>/Jenkinsfile
// Jenkins project configuration:
// Pipeline script from SCM:
//   SCM: Git
//   Repository URL: file:///C:/Users/gene/repos/cicd_class_1.git
//   Script Path: Jenkinsfile

// Jenkinsfile content:
pipeline {
    environment {
        TOOL_DIR = "${WORKSPACE}/cicd_tools"
    }
    stages {
        stage('Build') { ... }
    }
}
```

**Benefits:** "CI/CD tools under same source control as software. Coordinated changes easier. History maintained together."

**Related Concepts:**
- Pipeline Definition
- Source Control
- Infrastructure as Code
- Version Control

---

### Pipeline Steps

**Definition:**
Individual commands or actions within a pipeline stage. Steps are the actual work being performed - running scripts, invoking tools, publishing results, etc.

**Why It's Important:**
Steps are where the actual automation happens. Each step should be focused and clear. Failed steps typically fail the containing stage, stopping pipeline progression.

**How It's Used in CI/CD:**
Common steps in the course: `bat` (execute Windows batch command), `junit` (publish test results), `emailext` (send email). Steps call the lower-level automation scripts that do the real work.

**Example from Gene's Course:**
```groovy
stage('Test Debug') {
    steps {
        // Step 1: Run test script
        bat "python3 base_hilt.py --tver ${BUILD_ID} --com_dut ${COM_DUT} --com_sim ${COM_SIM}"

        // Step 2: Publish test results to Jenkins
        junit 'test_results*.xml'

        // Step 3: Verify no test failures
        bat "check_test_results.bat test_results*.xml"
    }
}
```

Each step has specific purpose. If any step fails (non-zero exit code), stage fails.

**Related Concepts:**
- Pipeline Stages
- Jenkinsfile
- Build Scripts
- Shell Integration

---

### Post-Build Actions

**Definition:**
Actions that execute after the main pipeline completes, with different actions triggered based on build result (success, failure, unstable, etc.). Part of the `post` section in declarative pipelines.

**Why It's Important:**
Enables conditional behavior based on outcomes - deliver successful builds, notify on failures, cleanup resources. Separates normal flow from exception handling and completion activities.

**How It's Used in CI/CD:**
Course pipeline has two post-build actions: on success, runs deliver.bat to copy images to release store; on unsuccessful (failure or unstable), sends email notification to developers.

**Example from Gene's Course:**
```groovy
post {
    success {
        // Only if all stages passed
        bat "deliver.bat"
        // Copies image files + other artifacts to release store
    }

    unsuccessful {
        // If any stage failed or unstable
        emailext subject: '$PROJECT_NAME - Build # $BUILD_NUMBER - $BUILD_STATUS!',
                 body: '$DEFAULT_CONTENT',
                 to: 'geneschrader.cicdclass@gmail.com'
    }
}
```

**Options:** `always`, `success`, `failure`, `unstable`, `unsuccessful`, `changed`, etc.

**Related Concepts:**
- Pipeline Definition
- Email Notifications
- Artifact Delivery
- Error Handling

---

### Build Triggers and Web Hooks

**Definition:**
Mechanisms that automatically start builds in response to events:
- **Build Triggers**: Jenkins configuration that defines what events start builds
- **Web Hooks**: HTTP callbacks that notify Jenkins when events occur (like Git push)

**Why It's Important:**
Automation eliminates manual build initiation, provides immediate feedback on code changes, enables true continuous integration, and ensures no commits go untested.

**How It's Used in CI/CD:**
Git post-update hook sends HTTP request to Jenkins via curl. Jenkins Generic Web Hook Trigger plugin receives request, matches token to project, and starts build. Developer's push immediately triggers automation.

**Example from Gene's Course:**
```bash
# Git hook: cicd_class_1.git/hooks/post-update
#!/bin/bash
curl -u gene:password http://localhost:8080/generic-webhook-trigger/invoke?token=cicd_class_1

# Jenkins project configuration:
# Build Triggers:
#   - Generic Web Hook Trigger
#   - Token: cicd_class_1

# Workflow:
# Developer: git push
# Git: runs post-update hook
# Hook: curl to Jenkins
# Jenkins: starts build
# Developer: sees build start in dashboard
```

**Related Concepts:**
- Git Hooks
- Automation
- Continuous Integration
- Jenkins Plugins

---

### Parameterized Builds

**Definition:**
Jenkins builds that accept parameters (inputs) that customize build behavior. Parameters have names, types, default values, and descriptions.

**Why It's Important:**
Enable flexible builds without hard-coding values, support different hardware configurations, allow manual builds with different settings, and document what can be customized.

**How It's Used in CI/CD:**
Course uses parameters for hardware configuration: COM ports for DUT and SIM boards, ST-Link serial number. Default values match instructor's setup. Jenkins provides parameters as environment variables to pipeline.

**Example from Gene's Course:**
```groovy
// Jenkins project configuration:
// This project is parameterized:
//   String Parameter:
//     Name: COM_DUT
//     Default: COM13
//     Description: COM port for device under test
//   String Parameter:
//     Name: COM_SIM
//     Default: COM15
//   String Parameter:
//     Name: STLINK_SN
//     Default: 066DFF535150898367092722

// Jenkinsfile uses:
bat "python3 base_hilt.py --com_dut ${COM_DUT} --com_sim ${COM_SIM}"
bat "flash.bat Debug ${STLINK_SN}"
```

**Related Concepts:**
- Jenkins Configuration
- Hardware Configuration
- Build Flexibility
- Environment Variables

---

### Build Status (Success, Unstable, Failure)

**Definition:**
The overall result of a build:
- **Success**: All stages passed
- **Unstable**: Build completed but quality gates violated (e.g., test failures, static analysis issues)
- **Failure**: Build process failed (compilation error, test crash, etc.)

**Why It's Important:**
Provides at-a-glance understanding of build health, enables different handling of different severities, supports quality metrics and trends, and guides developer response.

**How It's Used in CI/CD:**
Jenkins displays status with colors (green=success, yellow=unstable, red=failure). Static analysis failures mark build unstable but don't stop pipeline. Test failures mark build failed. Email notifications use status.

**Example from Gene's Course:**
```groovy
// Static analysis: failure makes build unstable, not failed
stage('Static Analysis') {
    steps {
        catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
            bat "static_analysis.bat"
            // If fails: stage=FAILURE, build=UNSTABLE
        }
    }
}

// Test failure: makes build failed
stage('Test Debug') {
    steps {
        bat "check_test_results.bat"
        // If fails: stage=FAILURE, build=FAILURE
    }
}
```

**Dashboard colors:** Green (success), Yellow (unstable), Red (failure)

**Related Concepts:**
- Quality Gates
- Pipeline Stages
- Build Results
- Notifications

---

### Jenkins Workspace

**Definition:**
A directory on the Jenkins build machine where Jenkins checks out code and performs builds. Each Jenkins project typically has its own workspace directory.

**Why It's Important:**
Provides isolated environment for builds, prevents builds from interfering with each other, serves as working directory for build scripts, and is automatically managed by Jenkins.

**How It's Used in CI/CD:**
Jenkins fetches code from Git into workspace before build. WORKSPACE environment variable points to this directory. Build scripts use ${WORKSPACE} to find source files and scripts.

**Example from Gene's Course:**
```groovy
// Jenkinsfile uses WORKSPACE variable:
environment {
    TOOL_DIR = "${WORKSPACE}/cicd_tools"
}

stages {
    stage('Build') {
        steps {
            bat "build.bat Debug all ${WORKSPACE} ..."
        }
    }
}

// Workspace typically something like:
// C:\Jenkins\workspace\cicd_class_1\
//   ├── Jenkinsfile
//   ├── app/
//   ├── cicd_tools/
//   ├── Core/
//   └── ...
```

**Related Concepts:**
- Jenkins Architecture
- Build Environment
- Environment Variables
- Source Checkout

---

### Lean Jenkinsfile Approach

**Definition:**
A design philosophy where the Jenkinsfile contains minimal logic and primarily calls lower-level scripts to perform actual work. The Jenkinsfile is a thin orchestration layer.

**Why It's Important:**
Reduces dependence on Jenkins-specific syntax, makes logic more portable and testable, enables running scripts manually outside Jenkins, simplifies Jenkins usage, and reduces learning curve.

**How It's Used in CI/CD:**
Jenkinsfile stages call batch scripts (build.bat, flash.bat, static_analysis.bat, deliver.bat) and Python scripts (base_hilt.py). Complex logic lives in these scripts, not in Jenkinsfile. Scripts can be tested independently.

**Example from Gene's Course:**
```groovy
// Jenkinsfile keeps it simple:
stage('Build') {
    steps {
        // Just call script, don't implement build logic here
        bat "build.bat Debug all ${WORKSPACE} ..."
    }
}

// NOT like this (bad):
stage('Build') {
    steps {
        // Don't put complex logic in Jenkinsfile
        bat "cd ${WORKSPACE}/Debug"
        bat "set PATH=..."
        bat "make clean"
        bat "make all"
        // Too much detail!
    }
}
```

Instructor: "Keep Jenkinsfile lean. Just calls lower-level scripts. More logic in your own scripts. Less Jenkins script language to learn."

**Related Concepts:**
- Separation of Concerns
- Build Scripts
- Pipeline Design
- Maintainability

---

## Category 5: Code Quality & Analysis Best Practices

### Static Code Analysis

**Definition:**
Automated examination of source code without executing it, using tools to find potential bugs, code smells, security vulnerabilities, style violations, and other issues.

**Why It's Important:**
Catches bugs that compilers miss, provides quick feedback before runtime testing, improves code quality proactively, documents coding standards enforcement, and reduces cost of bug fixes by finding issues early.

**How It's Used in CI/CD:**
cpp-check tool runs on application code after build stage. Analyzes C source files for issues like null pointer dereference, memory leaks, uninitialized variables. Failures mark build as unstable but don't stop pipeline.

**Example from Gene's Course:**
```c
// Code that compiles but has bug:
void dummy(void) {
    int *ptr = NULL;
    *ptr = 5;  // NULL pointer dereference
}
// Compiler: No warning
// cpp-check: "Null pointer dereference" - CAUGHT!

// Pipeline stage:
stage('Static Analysis') {
    steps {
        catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
            bat "static_analysis.bat"
        }
    }
}
```

**Pipeline Behavior:** "Does NOT stop pipeline. Marks build as 'unstable'. Continues to next stages. Allows flexibility."

**Related Concepts:**
- cpp-check Tool
- Code Quality
- Compiler Warnings
- Quality Gates

---

### Code Review

**Definition:**
Systematic examination of source code by humans to find bugs, improve code quality, share knowledge, and ensure standards compliance. Can be formal (inspections) or informal (peer review).

**Why It's Important:**
Catches issues tools miss (design problems, logic errors, maintainability issues), spreads knowledge across team, enforces standards and best practices, and improves code quality through collaboration.

**How It's Used in CI/CD:**
Course mentions code review as a form of static code analysis done by humans (vs. tools). Not explicitly implemented in the pipeline, but instructor notes "When done by person → code review."

**Example from Gene's Course:**
```c
// Static code analysis types:
// - When done by tool → static code analysis (cpp-check)
// - When done by person → code review

// Code review might catch:
// - Bad variable names
// - Missing error handling
// - Poor algorithm choice
// - Design issues
// Things tools can't easily detect
```

**Modern Practice:** Code review often integrated via pull requests before merging to main branch.

**Related Concepts:**
- Static Code Analysis (tool-based)
- Pull Requests
- Code Quality
- Team Collaboration

---

### cpp-check Tool

**Definition:**
A free, open-source static analysis tool for C/C++ that detects bugs and dangerous code patterns without executing code. Checks for memory leaks, null pointers, array bounds, and many other issues.

**Why It's Important:**
Provides automated code quality checking, free alternative to expensive tools, easy to integrate into scripts and CI/CD, and catches many common C/C++ pitfalls.

**How It's Used in CI/CD:**
static_analysis.bat script invokes cpp-check on application source files (not IDE-provided or third-party code). Runs after build stage. Output logged. Failures mark build unstable but allow pipeline to continue.

**Example from Gene's Course:**
```batch
REM static_analysis.bat
cppcheck --enable=all --error-exitcode=1 ..\app\

REM Finds issues like:
REM - Null pointer dereference
REM - Memory leaks
REM - Uninitialized variables
REM - Dead code
REM - etc.

REM Jenkins stage:
REM If cpp-check finds issues: exit code 1
REM catchError makes build UNSTABLE
REM Pipeline continues
```

**Alternative Mentioned:** Coverity (licensed, "incredible job at analyzing code")

**Related Concepts:**
- Static Code Analysis
- Code Quality
- Coverity (alternative tool)
- Quality Gates

---

### Coverity (Static Analysis Tool)

**Definition:**
A commercial static analysis tool (now part of Synopsys) that performs deep code analysis to find defects, security vulnerabilities, and quality issues in C, C++, and many other languages.

**Why It's Important:**
Industry-leading analysis capabilities, finds complex issues through deep semantic analysis, widely trusted in safety-critical and security-sensitive domains, and provides comprehensive reporting.

**How It's Used in CI/CD:**
NOT used in the course (cpp-check used instead due to cost). Instructor mentions it as alternative with very high capabilities: "incredible job at analyzing code."

**Example from Gene's Course:**
```c
// Instructor comment:
// "Alternative mentioned: Coverity (licensed, 'incredible job at analyzing code')"

// Coverity would be used similarly:
// coverity-analysis --build make all
// coverity-analyze --all
// coverity-commit-defects --url server

// But course uses free cpp-check instead
```

**Trade-off:** Coverity more powerful but requires license. cpp-check free and sufficient for course needs.

**Related Concepts:**
- cpp-check (free alternative)
- Static Code Analysis
- Commercial Tools
- Cost vs Capability

---

### False Positives and Suppression

**Definition:**
- **False Positive**: When static analysis tool reports an issue that isn't actually a problem in context
- **Suppression**: Technique to tell the tool to ignore specific warnings using special comments or configuration

**Why It's Important:**
Too many false positives reduce tool value (crying wolf), waste developer time investigating non-issues, and can lead to ignoring the tool. Suppression allows using tool effectively while handling legitimate false positives.

**How It's Used in CI/CD:**
When cpp-check reports false positives, can add special comments before code to suppress specific warnings. Only use when necessary - sometimes better to modify code to make tool happy.

**Example from Gene's Course:**
```c
// Suppressing false positive:
// Fault handling code doing address manipulations
// cppcheck-suppress comparePointers
void fault_handler() {
    uint32_t *stack_ptr = ...;
    // Intentional pointer manipulation
}

// Alternative: Modify code to avoid false positive
// Often better approach
```

**Instructor Guidance:** "Only use when necessary. Sometimes better to modify code to make cpp-check happy."

**Related Concepts:**
- Static Code Analysis
- cpp-check Tool
- Code Quality
- Tool Configuration

---

### Compiler Warnings vs Static Analysis

**Definition:**
Two different approaches to finding code problems:
- **Compiler Warnings**: Issues found during compilation (unused variables, type mismatches, etc.)
- **Static Analysis**: Deeper code examination by specialized tools (logic errors, security issues, etc.)

**Why It's Important:**
They're complementary - compilers focus on syntax and basic semantics, static analysis tools examine deeper patterns and logic. Both should be used. Static analysis catches issues compilers miss.

**How It's Used in CI/CD:**
Build stage captures compiler warnings. Static analysis stage runs cpp-check for deeper analysis. Example: NULL pointer dereference compiled without warning but cpp-check caught it.

**Example from Gene's Course:**
```c
// Example code:
void dummy(void) {
    int *ptr = NULL;
    *ptr = 5;
}

// Compiler: No warning or error
// - Compiles successfully
// - Legal C syntax
// - Will crash at runtime

// cpp-check: "Null pointer dereference"
// - Detected without running code
// - Prevented runtime crash
// - Caught in CI pipeline
```

**Instructor Demo:** Added bad code, pushed to Git, Jenkins built successfully, but static analysis caught the issue.

**Related Concepts:**
- Static Code Analysis
- Build Process
- Code Quality
- Compiler Capabilities

---

### Build Result: Stable vs Unstable

**Definition:**
Jenkins build classifications:
- **Stable**: Build passed completely, all quality gates passed
- **Unstable**: Build completed but quality concerns exist (test failures, analysis warnings, etc.)

**Why It's Important:**
Distinguishes between "broken" (can't build) and "questionable" (built but quality issues). Allows nuanced handling - unstable builds might be acceptable for development but not release.

**How It's Used in CI/CD:**
Static analysis failures create unstable builds rather than failed builds. This allows pipeline to continue testing while signaling quality issue. Dashboard shows unstable builds in yellow (vs green for stable, red for failed).

**Example from Gene's Course:**
```groovy
// Configure static analysis to create UNSTABLE not FAILURE:
stage('Static Analysis') {
    steps {
        catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
            bat "static_analysis.bat"
        }
    }
}

// Result:
// - Stage shows as FAILED (red)
// - But build overall is UNSTABLE (yellow)
// - Pipeline continues to next stages
// - Email still sent (unsuccessful includes unstable)
```

**Flexibility:** "Allows flexibility in handling minor issues."

**Related Concepts:**
- Build Status
- Quality Gates
- Static Code Analysis
- Pipeline Flow Control

---

### Code Quality Gates

**Definition:**
Criteria that code must meet to progress through the pipeline. Gates can be automated checks (tests pass, coverage threshold, analysis clean) or manual approvals.

**Why It's Important:**
Enforce quality standards, prevent bad code from progressing, provide objective quality measures, support continuous improvement, and reduce technical debt.

**How It's Used in CI/CD:**
Course implements quality gates: build must compile, static analysis results recorded (unstable if issues), tests must pass (fail build if not). Only successful builds delivered to release store.

**Example from Gene's Course:**
```groovy
// Implicit quality gates:
// 1. Code must compile (build stage fails if not)
// 2. Static analysis recorded (unstable if issues)
// 3. Software must flash successfully
// 4. Software must boot (console prompt test)
// 5. Version must match (build ID verification)
// 6. All tests must pass (fail build if not)

// Delivery only if all pass:
post {
    success {
        bat "deliver.bat"  // Quality gate: only successful builds
    }
}
```

Lesson 6 suggests more aggressive gate: prevent code from entering official branch until pipeline succeeds (requires very reliable tests).

**Related Concepts:**
- Build Status
- Test Results
- Static Analysis
- Continuous Delivery

---

## Category 6: Security & Production Readiness Best Practices

### Key-Based Authentication

**Definition:**
Authentication method using cryptographic key pairs (public/private keys) instead of passwords. Private key proves identity, public key verifies it. Much more secure than passwords.

**Why It's Important:**
More secure than passwords (keys are longer, randomly generated), not susceptible to dictionary attacks, can be revoked independently, no password transmission over network, and industry best practice for automated systems.

**How It's Used in CI/CD:**
Lesson 6 recommends key-based authentication for production systems. Should be used for Git access (SSH keys), Jenkins web hooks, and any automated system-to-system communication.

**Example from Gene's Course:**
```bash
# RECOMMENDED (not implemented in demo):
# Git with SSH key:
git remote add origin ssh://git@server/repo.git
# Uses ~/.ssh/id_rsa private key for authentication

# Web hook with key:
curl --key private.pem --cert cert.pem https://jenkins/webhook

# DEMO USED (insecure):
curl -u gene:password http://localhost:8080/...
# Username/password - OK for demo, NOT for production
```

**Production Improvement:** "Key-based authentication for web hook. No passwords in configurations."

**Related Concepts:**
- Certificate-Based Authentication
- Username/Password Authentication
- Security Best Practices
- Production Readiness

---

### Certificate-Based Authentication

**Definition:**
Authentication using digital certificates (X.509) that combine identity information with public keys, signed by a trusted certificate authority (CA). Used for both authentication and encryption.

**Why It's Important:**
Provides strong authentication, enables encrypted communication (TLS/SSL), scales well in organizations, allows fine-grained access control, and is required by many enterprise systems.

**How It's Used in CI/CD:**
Lesson 6 recommends certificate-based authentication for email notifications instead of username/password. Jenkins can use certificates for HTTPS connections to Git servers.

**Example from Gene's Course:**
```bash
# RECOMMENDED (not implemented in demo):
# Email with certificate:
# Jenkins Global Config:
#   Use SSL: Yes
#   SSL Certificate: /path/to/cert.pem
#   SSL Key: /path/to/key.pem

# DEMO USED (insecure):
# Gmail SMTP with username/password
# Requires "Less secure app access" enabled
# Google may disallow this in future
```

Instructor: "Should use certificates (instructor 'too lazy')."

**Related Concepts:**
- Key-Based Authentication
- Email Notifications
- Security Best Practices
- TLS/SSL

---

### Username/Password Authentication

**Definition:**
Authentication method where users provide username and password credentials. Simpler than key-based methods but less secure, especially when passwords are weak or reused.

**Why It's Important:**
Easy to understand and implement, but has security weaknesses: passwords can be guessed, stolen, or intercepted; often reused across systems; requires secure storage; and is discouraged for automated systems.

**How It's Used in CI/CD:**
Demo system uses username/password for Git web hooks and email notifications for simplicity. Lesson 6 identifies this as security weakness and recommends key/certificate-based authentication for production.

**Example from Gene's Course:**
```bash
# Git hook (demo approach - not secure):
curl -u gene:password http://localhost:8080/generic-webhook-trigger/invoke?token=cicd_class_1

# Email (demo approach - not secure):
# Gmail username: geneschrader.cicdclass@gmail.com
# Gmail password: (stored in Jenkins)
# Requires "Less secure app access" enabled

# PRODUCTION SHOULD USE:
# - Key-based authentication for web hooks
# - Certificate-based authentication for email
```

**Course Context:** "Fine for demo. Not realistic for production."

**Related Concepts:**
- Key-Based Authentication
- Certificate-Based Authentication
- Security Weaknesses
- Production Improvements

---

### ST-Link Serial Number

**Definition:**
A unique identifier assigned to each ST-Link debug probe/programmer. Used to specify which specific ST-Link device to use when multiple are connected to one computer.

**Why It's Important:**
Enables multi-board setups where multiple devices are programmed/tested from single host. Critical for HIL testing with DUT and simulation hardware. Prevents programming wrong board.

**How It's Used in CI/CD:**
Flash scripts specify ST-Link serial number to ensure correct board is programmed. Jenkins parameterized build has STLINK_SN parameter. Test script identifies which board is DUT vs SIM.

**Example from Gene's Course:**
```batch
REM List connected ST-Links:
STM32_Programmer_CLI -l
REM Output:
REM - SN: 066DFF535150898367092722 (Nucleo board)
REM - SN: 48FF... (Blue Pill with ST-Link adapter)

REM Flash specific board:
STM32_Programmer_CLI -c port=SWD sn=066DFF535150898367092722 -d Debug/cicd_class_1.elf 0x08000000

REM Jenkins parameter:
# Name: STLINK_SN
# Default: 066DFF535150898367092722
# Used in: flash.bat ${STLINK_SN}
```

**Related Concepts:**
- Flash Programming
- Hardware Configuration
- Parameterized Builds
- Multi-Board Setup

---

### Jenkins Credentials

**Definition:**
Jenkins feature for securely storing and managing sensitive information (passwords, API tokens, SSH keys, certificates) used by builds and pipelines.

**Why It's Important:**
Centralizes credential management, keeps secrets out of Jenkinsfiles and source code, supports encryption at rest, enables credential rotation, and provides audit trail.

**How It's Used in CI/CD:**
Demo course stores credentials directly in configuration (simple but not secure). Production systems should use Jenkins Credentials store to manage Git credentials, email passwords, API tokens, etc.

**Example from Gene's Course:**
```groovy
// DEMO APPROACH (credentials in clear):
// Web hook:
curl -u gene:password ...

// Email in global config:
// Username: geneschrader.cicdclass@gmail.com
// Password: (stored in plain text)

// PRODUCTION APPROACH (not shown):
// Store in Jenkins Credentials
// Reference in Jenkinsfile:
withCredentials([usernamePassword(
    credentialsId: 'git-credentials',
    usernameVariable: 'GIT_USER',
    passwordVariable: 'GIT_PASSWORD'
)]) {
    // Use ${GIT_USER} and ${GIT_PASSWORD}
}
```

**Related Concepts:**
- Security Best Practices
- Secret Management
- Jenkins Configuration
- Production Readiness

---

### Production Readiness

**Definition:**
The state where a system, process, or code is suitable for deployment in a production environment. Includes reliability, security, scalability, maintainability, and supportability considerations.

**Why It's Important:**
Production environments have different requirements than demos: higher reliability needs, security requirements, scalability demands, compliance requirements, and support considerations. Demo systems often cut corners that production cannot.

**How It's Used in CI/CD:**
Lesson 6 explicitly addresses "Next Steps" to move from demo system to production-grade CI/CD: network-based repos, security improvements, branch protection, and team collaboration features.

**Example from Gene's Course:**
```
Demo System:
- File-based Git repo (local)
- Username/password authentication
- Single developer
- Manual deployment
- Simple hardware setup

Production Requirements:
- Network Git server or GitHub
- Key/certificate authentication
- Multiple developers
- Automated deployment with approvals
- Redundant hardware
- Monitoring and alerting
- Backup and disaster recovery
- Documentation and runbooks
```

Instructor: "Demo system works but has limitations for real-world use."

**Related Concepts:**
- Security Improvements
- Scalability
- Multi-Developer Support
- Lesson 6 Topics

---

### Network Security

**Definition:**
Practices and technologies that protect network infrastructure and data transmitted over networks from unauthorized access, misuse, or theft. Includes authentication, encryption, firewalls, and access control.

**Why It's Important:**
Protects intellectual property (source code), prevents unauthorized access to build systems, ensures integrity of software releases, and meets compliance requirements.

**How It's Used in CI/CD:**
Lesson 6 discusses moving from file-based local repo to network-based Git server with secure interfaces. Web hooks through firewalls require special consideration. SSH or HTTPS with certificates recommended.

**Example from Gene's Course:**
```
# Demo (no network security needed):
file:///C:/Users/gene/repos/cicd_class_1.git
# Everything on localhost

# Production (network security important):
ssh://git@git.company.com/repos/cicd_class_1.git
# - SSH protocol (encrypted)
# - Key-based authentication
# - Firewall rules
# - Access control lists

# GitHub option:
https://github.com/company/cicd_class_1.git
# - HTTPS (encrypted)
# - Token or key authentication
# - GitHub's security infrastructure
# - Web hook through firewall challenges
```

**Related Concepts:**
- Key-Based Authentication
- Production Readiness
- Git Server Options
- Firewalls

---

### Less Secure App Access (Gmail)

**Definition:**
A Google Account security setting that allows apps to access Gmail using username/password authentication instead of OAuth2. Google considers this less secure and may disallow it.

**Why It's Important:**
Example of security trade-offs: easier to implement but less secure. Demonstrates why instructor calls demo approach "too lazy" - it works but isn't production-grade security.

**How It's Used in CI/CD:**
Demo system uses this setting to allow Jenkins to send email via Gmail SMTP. Instructor acknowledges this is insecure and recommends certificate-based authentication for production.

**Example from Gene's Course:**
```
# Gmail configuration required for demo:
# 1. Go to Google Account settings
# 2. Security section
# 3. Turn ON "Less secure app access"
# 4. Jenkins can now use SMTP with username/password

# Gmail warning:
# "Some apps and devices use less secure sign-in technology,
#  which makes your account more vulnerable."

# Instructor comment:
# "NOTE: Google may disallow this in future"
# "Should use: Certificates (instructor 'too lazy')"
```

**Production Alternative:** OAuth2, App Passwords, or certificate-based authentication.

**Related Concepts:**
- Email Notifications
- Certificate-Based Authentication
- Security Trade-offs
- Production vs Demo

---

## Category 7: Team Collaboration & Workflow Best Practices

### Multi-Developer Workflow

**Definition:**
The processes, patterns, and practices that enable multiple developers to work on the same codebase effectively, managing concurrent changes, integration, and collaboration.

**Why It's Important:**
Most real-world projects involve teams. Poor multi-developer workflows lead to conflicts, lost work, integration problems, and productivity loss. Good workflows enable parallel development with minimal friction.

**How It's Used in CI/CD:**
Lesson 6 addresses multi-developer considerations. Multiple developers each have local repos, push to shared remote repo at various times. Jenkins fetches and tests. Issues arise with IDE settings files and makefile management.

**Example from Gene's Course:**
```
Architecture:
Git Server (Official Repo)
    ↑ push          ↓ fetch
Developer 1 Repo ←→ Developer 2 Repo
    ↓
Jenkins Repo (read-only)

Workflow:
1. Dev 1: work locally, commit, push
2. Git server: runs post-update hook
3. Jenkins: fetches, builds, tests
4. Dev 2: pulls Dev 1's changes, works locally
5. Dev 2: commits, pushes
6. Jenkins: fetches, builds, tests
7. Repeat...
```

**Challenges:** IDE settings files with local paths, makefile management, merge conflicts.

**Related Concepts:**
- Whipsawed Files
- Standardized Paths
- Layered Repository Architecture
- Team Collaboration

---

### Whipsawed Files

**Definition:**
Files that appear to constantly change in Git history even though their logical content is identical, due to non-deterministic ordering or formatting. Causes confusing history where files "flip-flop" between developers' versions.

**Why It's Important:**
Pollutes Git history with meaningless changes, makes tracking real changes difficult, creates unnecessary merge conflicts, and signals poor tool/workflow design.

**How It's Used in CI/CD:**
Lesson 6 identifies IDE settings files as prone to whipsawing: same logical settings but different line order causes Git to see different files. Each developer's commit includes "changed" settings file even though settings identical.

**Example from Gene's Course:**
```
# Problem scenario:
# Developer A's .settings file:
setting1=value1
setting2=value2
setting3=value3

# Developer B's .settings file (logically identical):
setting1=value1
setting3=value3
setting2=value2

# Git perspective:
# 1. Dev A pushes: file changes from B's version to A's
# 2. Dev B pulls, makes unrelated change, pushes: file changes back
# 3. Dev A pulls, makes unrelated change, pushes: file changes again
# 4. History shows constant "changes" to settings file
# 5. Real changes buried in noise
```

**Impact:** "May work, but not ideal. Confusing Git history. Hard to track real changes."

**Related Concepts:**
- IDE Settings Files
- Git History
- Team Collaboration
- Layered Repository Architecture (solution)

---

### Layered Repository Architecture

**Definition:**
A repository organization strategy where project files are separated into multiple Git repositories based on their characteristics: application code, IDE-maintained files, and per-developer settings.

**Why It's Important:**
Solves multi-developer IDE challenges by isolating files with different change patterns, prevents whipsawed files, enables sharing application code across projects, and allows per-developer customization without polluting shared history.

**How It's Used in CI/CD:**
Lesson 6 proposes three-layer architecture:
- Layer 1: Application code (hand-written, identical for all)
- Layer 2: IDE-maintained files (generated, needed for CI/CD)
- Layer 3: Per-developer settings (may differ, not needed for CI/CD)

Each layer in separate repository with appropriate sharing strategy.

**Example from Gene's Course:**
```
# Three-layer structure:

# Layer 1: application_code.git
app/
├── gpioapp/
│   ├── appmain.c
│   └── commands.c
└── common/
# Shared across all developers and projects
# May appear in multiple IDE projects

# Layer 2: cicd_build_files.git
Core/              # Generated initialization code
Drivers/           # IDE-supplied libraries
Debug/makefile     # Generated makefiles
Release/makefile
.ioc               # Hardware configuration
# Shared, identical for all developers
# Required for CI/CD builds

# Layer 3: developer_settings.git (or per-dev folders)
.settings/         # IDE project settings
.project
.cproject
# Per-developer, may differ
# NOT needed for CI/CD builds
```

**Key Requirement:** Each developer must ensure their settings generate same code in Layer 2.

**Related Concepts:**
- Whipsawed Files (problem solved)
- Multi-Developer Workflow
- IDE Settings Files
- Git Submodules/Subtrees

---

### Standardized Paths

**Definition:**
A team convention where all developers use identical directory structures and paths for project files, ensuring file-based references remain consistent across machines.

**Why It's Important:**
Simple solution to IDE settings file problems, eliminates path-related differences in settings files, easy to implement and understand, and requires minimal technical complexity.

**How It's Used in CI/CD:**
Lesson 6 suggests simple solution for multi-developer IDE issues: all developers must put IDE project folder at same location (e.g., C:\embedded\cicd_class_1). Path names in settings files then identical for everyone.

**Example from Gene's Course:**
```
# Problem:
# Dev A: C:\Users\alice\workspace\cicd_class_1
# Dev B: C:\Users\bob\my_projects\cicd_class_1
# Settings files differ due to paths

# Solution: Team convention:
# All developers use: C:\embedded\cicd_class_1

# .mxproject file then identical:
project.path=C:\embedded\cicd_class_1

# Benefits:
# - Simple to implement
# - No whipsawed files
# - No special tooling needed

# Drawback:
# - Inflexible
# - Doesn't solve all IDE settings issues
```

Instructor: "Fixed location for all developers. Project convention."

**Related Concepts:**
- Multi-Developer Workflow
- Whipsawed Files
- IDE Settings Files
- Team Conventions

---

### Developer Workflow

**Definition:**
The sequence of steps and practices an individual developer follows when making changes: write code, build, test, commit, and push. Includes both technical steps and timing/discipline aspects.

**Why It's Important:**
Proper workflow prevents bugs from reaching CI/CD, maintains repository quality, ensures builds stay working, and supports team collaboration. Bad workflow causes CI/CD failures and wastes team time.

**How It's Used in CI/CD:**
Course defines specific workflow to accommodate makefile management: make changes, build/test in IDE, iterate as needed, then **crucial step**: build BOTH Debug AND Release in IDE before committing to ensure makefiles current.

**Example from Gene's Course:**
```
# Recommended workflow:
1. Make code changes in IDE project
2. Build first to fix compiler problems
3. Test and debug using IDE
4. Iterate: make more changes, test/debug (loop as needed)
5. When code ready to push to Git for CI/CD:
   ★ Do final IDE builds for BOTH Debug AND Release ★
   (ensures makefiles up-to-date)
6. Git commit file changes (both code AND makefiles)
7. Push commit to remote repo
8. Jenkins automatically builds and tests

# Common mistake:
# Only build Debug before committing
# → Release makefiles outdated
# → Jenkins Release build fails

# Result: "Workable system even if not ideal"
```

**Related Concepts:**
- Makefile Management
- Build Configurations
- Multi-Developer Workflow
- CI/CD Pipeline

---

### Code Synchronization

**Definition:**
The process of keeping multiple repository copies (different developers, Jenkins) updated with latest changes through Git operations (push, pull, fetch, merge).

**Why It's Important:**
Prevents developers from working on stale code, reduces merge conflicts, ensures CI/CD tests latest code, and maintains consistency across team.

**How It's Used in CI/CD:**
Developers push changes to official repo. Jenkins fetches updates. Other developers pull to stay current. Frequency affects integration complexity - more frequent is better (continuous integration).

**Example from Gene's Course:**
```bash
# Developer A workflow:
git pull           # Get latest from team
# Make changes
git add .
git commit -m "..."
git push           # Share with team

# Developer B workflow:
git pull           # Get A's changes
# Make changes
git add .
git commit -m "..."
git push

# Jenkins workflow (automatic):
# Triggered by push webhook
git fetch origin master
git checkout master
# Build, test, analyze

# Timing:
# Course: Single developer, pushes when ready
# Real team: Multiple developers push frequently
# CI: Tests every push (continuous)
```

**Related Concepts:**
- Multi-Developer Workflow
- Git Operations
- Merge Conflicts
- Continuous Integration

---

## Category 8: Tools & Technologies Reference

This section provides a comprehensive reference of all tools and technologies used in the course, organized by purpose.

---

### STM32CubeIDE

**Definition:**
A free, Eclipse-based integrated development environment from STMicroelectronics for developing STM32 microcontroller applications. Includes code editor, debugger, build tools, code generator, and hardware configuration tools.

**Why It's Important:**
Official STM development tool, provides complete development environment, generates initialization code from graphical configuration, includes ARM GCC toolchain, and is widely used in embedded development.

**How It's Used in CI/CD:**
Used for project creation, hardware configuration, application development, and debugging. Generates makefiles that enable command-line builds. Course demonstrates CI/CD is possible even with GUI-based IDEs through proper automation.

**Example from Gene's Course:**
```
# IDE Used For:
# - Creating project (Nucleo F401RE board)
# - Configuring pins/peripherals (.ioc file)
# - Generating initialization code
# - Writing application code
# - Building (generates makefiles)
# - Debugging with ST-Link
# - Local testing

# CI/CD Uses:
# - Makefiles generated by IDE
# - Tool chain installed with IDE
# - NOT the IDE itself (just tools)

# Key Insight:
# "CI/CD is possible even with GUI-based IDEs"
```

**Course Philosophy:** "Demonstrates that CI/CD is possible even with GUI-based IDEs through proper scripting and workflow design."

**Related Concepts:**
- Eclipse IDE (base platform)
- Build Automation
- Tool Chain
- Headless Builds (considered but not used)

---

### Eclipse IDE

**Definition:**
An open-source, extensible integrated development environment with plugin architecture. Originally for Java, now supports many languages. Base platform for STM32CubeIDE and many other IDEs.

**Why It's Important:**
Provides robust IDE infrastructure, extensive plugin ecosystem, widely known and used, and enables STM's customized embedded development environment.

**How It's Used in CI/CD:**
STM32CubeIDE is built on Eclipse platform. Eclipse provides project management, editor, build integration. STM adds MCU-specific plugins for hardware configuration, code generation, debugging.

**Example from Gene's Course:**
```
# STM32CubeIDE components:
# - Eclipse IDE base (infrastructure)
#   - Project/workspace management
#   - Editor, build integration
#   - Plugin system
# - STM32 plugins:
#   - MCU/board database
#   - .ioc editor (graphical config)
#   - Code generator
#   - ST-Link debugging support
#   - Build configurations
```

**Related Concepts:**
- STM32CubeIDE
- Plugin Architecture
- IDE Project Structure
- Workspace

---

### Git

**Definition:**
A distributed version control system that tracks changes to files, enables collaboration, maintains history, and supports branching/merging. Industry standard for source code management.

**Why It's Important:**
Enables version control, collaboration, history tracking, and branching. Foundation of modern CI/CD. Distributed nature provides flexibility and offline capability.

**How It's Used in CI/CD:**
Core of the system: developers use for local version control, push to remote repo triggers CI/CD, Jenkins fetches code to build. All commands done via command line (not IDE plugin) for transparency.

**Example from Gene's Course:**
```bash
# Developer operations:
git init
git add .
git commit -m "..."
git push

# Remote repo operations:
git init --bare
# Receive pushes
# Run post-update hook

# Jenkins operations:
git fetch origin master
git checkout master

# Key commands covered:
# - git status -u
# - git diff
# - git log
# - git remote add
```

Instructor: "Important to know Git at command line. Course is about moving from GUI to command line."

**Related Concepts:**
- Source Control
- Remote Repository
- Git Hooks
- GitHub (alternative hosting)

---

### GitHub

**Definition:**
A web-based hosting service for Git repositories, owned by Microsoft. Provides additional features beyond Git: pull requests, issue tracking, project management, CI/CD (GitHub Actions), and social coding features.

**Why It's Important:**
Industry-standard for hosting open-source and private projects, provides collaboration features, integrated CI/CD capabilities, and is widely used in job requirements.

**How It's Used in CI/CD:**
Lesson 6 discusses using GitHub as alternative to local file-based repo for production. Provides secure hosting, web interface, pull request workflow. Challenges include web hook triggers through firewalls and HIL testing with GitHub builds.

**Example from Gene's Course:**
```bash
# Demo uses:
file:///C:/Users/gene/repos/cicd_class_1.git

# Production could use:
https://github.com/company/cicd_class_1.git
# Or:
git@github.com:company/cicd_class_1.git

# Benefits:
# - Professional hosting
# - Pull request workflow
# - Access control
# - Backup/redundancy

# Challenges:
# - Web hook trigger through firewall
# - How to do HIL testing with GitHub builds?
# - May have CI/CD capabilities (investigate for embedded)
```

**Course Resources:** "GitHub repos available for Nucleo board project and Blue Pill HIL simulation hardware."

**Related Concepts:**
- Git
- Remote Repository
- Production Improvements
- Pull Requests

---

### Jenkins

**Definition:**
An open-source automation server that orchestrates software builds, tests, and deployments. Plugin-based architecture supports extensive customization. Industry-standard for CI/CD implementation.

**Why It's Important:**
Provides automation hub for CI/CD, visualizes pipeline execution, stores build history and test results, triggers automated workflows, and is widely used in industry.

**How It's Used in CI/CD:**
Jenkins is the build server: receives web hook triggers, fetches code from Git, executes pipeline defined in Jenkinsfile, runs build/test/analysis scripts, stores results, and sends notifications.

**Example from Gene's Course:**
```groovy
# Jenkins provides:
# - HTTP interface (localhost:8080)
# - Web hook endpoint (Generic Web Hook Trigger plugin)
# - Pipeline execution engine
# - Workspace management
# - Build history and trends
# - Test result visualization
# - Email notifications
# - Environment variables (BUILD_ID, WORKSPACE)

# Jenkinsfile orchestrates:
pipeline {
    stages {
        stage('Build') { ... }
        stage('Test') { ... }
    }
}
```

Instructor: "Sophisticated script runner and data store specialized for DevOps."

**Related Concepts:**
- Jenkinsfile
- Pipeline
- Jenkins Plugins
- Build Server

---

### Jenkins Plugins

**Definition:**
Extensions that add functionality to Jenkins. Plugin architecture allows third parties to add features. Thousands of plugins available for various tools, SCM systems, notifications, etc.

**Why It's Important:**
Make Jenkins extensible and powerful, enable integration with specific tools and services, support various workflows and use cases, and are often created by tool vendors or community.

**How It's Used in CI/CD:**
Course uses specific plugins: Generic Web Hook Trigger (for Git hooks), Extended Email Notification (for failure alerts), JUnit (for test result display). Others installed by default.

**Example from Gene's Course:**
```
# Required plugins (install manually):
# - Generic Web Hook Trigger
#   - Receives HTTP webhook from Git
#   - Token-based project identification
# - Extended Email Notification
#   - Flexible email configuration
#   - Template support
# - JUnit
#   - Parses JUnit XML test results
#   - Displays trends and details

# Usage:
# In Jenkinsfile:
junit 'test_results*.xml'  # JUnit plugin
emailext ...               # Email plugin

# In project config:
# Build Triggers:
#   Generic Web Hook Trigger
#   Token: cicd_class_1
```

**Plugin Ecosystem:** "Good: third parties easily add features. Bad: more poking around, documentation sometimes inconsistent."

**Related Concepts:**
- Jenkins
- Web Hooks
- Email Notifications
- Test Reporting

---

### Python 3.x

**Definition:**
A high-level, interpreted programming language known for readability and extensive libraries. Version 3.x is current major version (incompatible with Python 2.7).

**Why It's Important:**
Excellent for scripting and automation, extensive library ecosystem (p-expect, junit_xml), readable code ideal for test scripts, and widely used in test automation.

**How It's Used in CI/CD:**
Python test scripts (base_hilt.py) automate HIL testing: control serial connections via p-expect, send commands to DUT and SIM boards, verify responses, and generate JUnit XML reports.

**Example from Gene's Course:**
```python
# base_hilt.py - Test automation script
import pexpect
from junit_xml import TestSuite, TestCase

# Connect to boards
dut = pexpect.spawn(plink + COM_DUT)
sim = pexpect.spawn(plink + COM_SIM)

# Data-driven tests
test_steps = [
    {'device': 'dut', 'command': 'write B 9 1', 'expect': 'OK'},
    {'device': 'sim', 'command': 'read B 9', 'expect': '1'}
]

# Execute and report
# Generates test_results_dut_debug.xml

# Called from Jenkins:
bat "python3 base_hilt.py --tver ${BUILD_ID} --com_dut ${COM_DUT}"
```

Instructor: "Python (instructor's favorite after C/C++)."

**Note:** "Python 3.x (not 2.7)" - important version distinction.

**Related Concepts:**
- p-expect Module
- junit_xml Module
- Test Automation
- Scripting

---

### Windows Batch Scripts

**Definition:**
Shell scripts for Windows command prompt (.bat or .cmd files) that automate command sequences. Simpler than PowerShell but sufficient for many automation tasks.

**Why It's Important:**
Provide automation on Windows, integrate with IDEs and tools that expect Windows environment, familiar to Windows developers, and enable CI/CD on Windows (course platform).

**How It's Used in CI/CD:**
All build automation scripts are Windows batch files: build.bat (invoke make), flash.bat (program flash), static_analysis.bat (run cpp-check), deliver.bat (copy releases), check_test_results.bat (verify tests passed).

**Example from Gene's Course:**
```batch
@echo off
setlocal

REM build.bat - Build automation
set CWD=%3\%1       # Config from argument
set PATH=%4;%PATH% # Toolchain from argument

REM Build ID injection
echo #ifndef VERSION_H > version.h
echo #define BUILD_VERSION "%5" >> version.h
echo #endif >> version.h

cd /d %CWD%
make -j4 %2  # Target from argument
```

**Jenkinsfile calls batch scripts:**
```groovy
bat "build.bat Debug all ${WORKSPACE} ..."
```

**Related Concepts:**
- Build Automation
- Bash (Linux alternative)
- Scripting
- Environment Variables

---

### Bash

**Definition:**
Unix shell and command language, standard shell on Linux and macOS. More powerful than Windows batch but requires Unix-like environment on Windows (Git Bash, WSL, Cygwin).

**Why It's Important:**
Standard for Unix/Linux automation, more powerful scripting capabilities than batch, used for Git hooks even on Windows (Git provides Bash interpreter).

**How It's Used in CI/CD:**
Git post-update hook is a Bash script (even on Windows - Git provides Bash interpreter). Uses curl to trigger Jenkins webhook.

**Example from Gene's Course:**
```bash
#!/bin/bash
# File: cicd_class_1.git/hooks/post-update
curl -u gene:password http://localhost:8080/generic-webhook-trigger/invoke?token=cicd_class_1
```

**Requirement:** `#!/bin/bash` shebang required. Git on Windows includes Bash interpreter.

**Contrast:** Build scripts are Windows Batch (native), Git hooks are Bash (Git provides).

**Related Concepts:**
- Git Hooks
- Windows Batch Scripts
- Shell Scripting
- Cross-Platform Considerations

---

### STM32_Programmer_CLI

**Definition:**
Command-line interface version of STM32 Cube Programmer, a tool for flashing (programming) STM32 microcontrollers via various interfaces (ST-Link, UART, USB, etc.).

**Why It's Important:**
Enables automated flash programming from scripts, no GUI required, supports all ST-Link features, and is essential for CI/CD flash programming step.

**How It's Used in CI/CD:**
flash.bat script invokes STM32_Programmer_CLI to program flash memory after successful build. Specifies image file, flash address, ST-Link serial number, and performs hardware reset after programming.

**Example from Gene's Course:**
```batch
REM List connected ST-Links:
STM32_Programmer_CLI -l

REM Flash programming:
STM32_Programmer_CLI -c port=SWD sn=%STLINK_SN% -d %IMAGE% 0x08000000 -hardRst

REM Breakdown:
REM -c port=SWD sn=<serial> : Connect via SWD with specific ST-Link
REM -d <image> <address>    : Download (program) image to address
REM -hardRst                : Hardware reset when done

REM Flash address:
REM 0x08000000 = start of flash for STM32 Cortex-M MCUs
```

**Installation:** "Comes with STM32 Cube Programmer (GUI tool - must download/install). CLI tool does everything needed."

**Related Concepts:**
- Flash Programming
- ST-Link
- Build Automation
- Hardware Reset

---

### cpp-check

**Definition:**
A free, open-source static analysis tool for C/C++ code. Detects bugs, undefined behavior, dangerous constructs, and style issues without executing code.

**Why It's Important:**
Free alternative to expensive tools, easy to integrate in scripts, finds common C/C++ issues, and provides automated code quality checking.

**How It's Used in CI/CD:**
static_analysis.bat script runs cpp-check on application source files. Finds issues like null pointer dereference, memory leaks, uninitialized variables. Failures mark build unstable but don't stop pipeline.

**Example from Gene's Course:**
```batch
REM static_analysis.bat
cppcheck --enable=all --error-exitcode=1 ..\app\

REM Finds issues:
REM [app/main.c:10]: Null pointer dereference
REM [app/gpioapp.c:42]: Memory leak: buffer
REM [app/commands.c:15]: Uninitialized variable: result

REM If issues found: exit code 1
REM Jenkins catchError: build marked UNSTABLE
REM Pipeline continues

REM Suppression:
REM // cppcheck-suppress nullPointer
```

**Alternative:** "Coverity (licensed, 'incredible job at analyzing code')"

**Related Concepts:**
- Static Code Analysis
- Code Quality
- False Positives
- Coverity

---

### p-expect (Python Module)

**Definition:**
A Python module for automating interactive console applications, based on the Expect tool. Allows sending commands to programs and verifying responses using pattern matching.

**Why It's Important:**
Ideal for serial communication testing, enables stimulus-response test automation, provides timeout handling and pattern matching, and is based on proven Expect tool from 1990s.

**How It's Used in CI/CD:**
Test script uses p-expect to control DUT and SIM boards via serial: spawn p-link instances, send commands with sendline(), verify responses with expect(), handle timeouts.

**Example from Gene's Course:**
```python
import pexpect

# Spawn serial connection
dut = pexpect.spawn(f'{plink} -serial {COM_DUT}')

# Send command
dut.sendline('write B 9 1')

# Verify response
dut.expect('OK', timeout=2)  # Pattern matching
# If pattern not found within timeout: exception raised

# Version verification
dut.sendline('version')
dut.expect(f'version={expected_version}', timeout=2)
```

Instructor: "Based on popular 'expect' tool (instructor used in 1990s)."

**Related Concepts:**
- Test Automation
- Stimulus-Response Testing
- Serial Communication
- p-link Tool

---

### p-link

**Definition:**
Command-line serial terminal program from the PuTTY suite. Allows connections to serial ports from command line, can be spawned and controlled by scripts.

**Why It's Important:**
Enables automated serial communication from scripts, provides reliable serial terminal functionality, works well with p-expect for test automation, and is scriptable unlike GUI terminal programs.

**How It's Used in CI/CD:**
Test script spawns p-link instances to connect to DUT and SIM serial ports. p-expect then controls these p-link instances to send commands and read responses.

**Example from Gene's Course:**
```python
# Spawn p-link serial connection
plink = 'path/to/plink.exe'
COM_DUT = 'COM13'
COM_SIM = 'COM15'

# Create connections
dut = pexpect.spawn(f'{plink} -serial {COM_DUT} -sercfg 115200,8,n,1')
sim = pexpect.spawn(f'{plink} -serial {COM_SIM} -sercfg 115200,8,n,1')

# Now can send/receive via dut and sim objects

# Jenkins passes COM ports as parameters:
bat "python3 base_hilt.py --com_dut ${COM_DUT} --com_sim ${COM_SIM}"
```

**From PuTTY Suite:** "Serial terminal program from PuTTY."

**Related Concepts:**
- p-expect Module
- Serial Communication
- Test Automation
- HIL Testing

---

### junit_xml (Python Module)

**Definition:**
A Python module for creating JUnit XML test result files programmatically. Provides classes for test suites, test cases, and failures with simple API.

**Why It's Important:**
Makes generating standard test result format easy, enables integration with Jenkins and other CI/CD tools, avoids manual XML generation, and handles proper formatting automatically.

**How It's Used in CI/CD:**
Test script creates TestCase objects for each test, adds to TestSuite, writes JUnit XML files. Jenkins junit step reads these files to display results.

**Example from Gene's Course:**
```python
from junit_xml import TestSuite, TestCase

# Create test cases
test_cases = []

tc = TestCase('test_console_prompt', classname='HIL', elapsed_sec=0.5)
test_cases.append(tc)

tc = TestCase('test_version', classname='HIL', elapsed_sec=1.0)
if version_mismatch:
    tc.add_failure_info('Version mismatch: expected jenkins-42, got jenkins-41')
test_cases.append(tc)

# Create suite and write file
ts = TestSuite("HIL Test Suite", test_cases)
with open('test_results_dut_debug.xml', 'w') as f:
    TestSuite.to_file(f, [ts])

# Jenkinsfile reads:
junit 'test_results*.xml'
```

**Output:** "19 tests, 0 disabled, 0 errors, 0 failures" or "19 tests, 5 failures"

**Related Concepts:**
- JUnit XML Format
- Test Reporting
- Jenkins Integration
- Test Automation

---

### curl

**Definition:**
Command-line tool for transferring data using various protocols (HTTP, HTTPS, FTP, etc.). Supports authentication, headers, POST data, and many other features. Ubiquitous in automation.

**Why It's Important:**
Standard tool for HTTP operations from scripts, enables web hook triggers, works across platforms, and is simple yet powerful.

**How It's Used in CI/CD:**
Git post-update hook uses curl to send HTTP request to Jenkins, triggering build via Generic Web Hook Trigger plugin.

**Example from Gene's Course:**
```bash
#!/bin/bash
# Git hook: post-update
curl -u gene:password http://localhost:8080/generic-webhook-trigger/invoke?token=cicd_class_1

# Breakdown:
# curl                  : HTTP client tool
# -u gene:password      : Basic authentication
# http://localhost:8080 : Jenkins server
# /generic-webhook-trigger/invoke : Plugin endpoint
# ?token=cicd_class_1   : Identifies project to build

# Result: Jenkins starts build immediately
```

**Production Improvement:** Use key-based authentication instead of username/password in URL.

**Related Concepts:**
- Web Hooks
- Git Hooks
- HTTP/HTTPS
- Build Triggers

---

### HAL (Hardware Abstraction Layer)

**Definition:**
STMicroelectronics' Hardware Abstraction Layer library for STM32 microcontrollers. Provides consistent, higher-level API for peripherals across different STM32 families, abstracting hardware details.

**Why It's Important:**
Simplifies peripheral usage, provides portability across STM32 families, includes extensive functionality, and is well-documented and supported by ST.

**How It's Used in CI/CD:**
Application code uses HAL UART library for serial communication, HAL GPIO functions for pin control. Choice between HAL and LL (low-level) libraries is project decision.

**Example from Gene's Course:**
```c
// HAL GPIO functions used in application:
HAL_GPIO_ReadPin(port, pin);
HAL_GPIO_WritePin(port, pin, value);

// HAL UART for console:
HAL_UART_Transmit(&huart2, buffer, length, timeout);
HAL_UART_Receive(&huart2, buffer, length, timeout);

// Initialization (generated by IDE):
HAL_UART_Init(&huart2);
HAL_GPIO_Init(port, &config);
```

**Software Architecture Note:** "Simple super loop. Uses STM32 HAL UART library. Made portable to other STM32 MCUs."

**Related Concepts:**
- LL (Low-Level) Drivers
- CMSIS
- STM32 Peripherals
- IDE-Provided Libraries

---

### LL (Low-Level) Drivers

**Definition:**
STMicroelectronics' Low-Level driver library - an alternative to HAL providing more direct access to hardware registers with less abstraction and lower overhead.

**Why It's Important:**
Offers better performance than HAL (less overhead), provides more control over hardware, suitable for time-critical code, but requires more hardware knowledge.

**How It's Used in CI/CD:**
Course uses HAL, but LL is alternative choice. Switching between HAL and LL requires IDE build to update makefiles (one of the changes that needs makefile regeneration).

**Example from Gene's Course:**
```c
// LL equivalent (not used in course):
LL_GPIO_SetOutputPin(port, pin);
LL_GPIO_ResetOutputPin(port, pin);
value = LL_GPIO_IsInputPinSet(port, pin);

// Closer to register access:
// HAL:  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
// LL:   LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_9);
// Direct: GPIOB->ODR |= (1 << 9);
```

**Trade-off:** HAL = easier to use, LL = better performance.

**Makefile Impact:** "Switching between HAL and LL libraries" requires IDE builds to regenerate makefiles.

**Related Concepts:**
- HAL
- Performance Optimization
- Hardware Access
- IDE-Provided Libraries

---

### CMSIS

**Definition:**
Cortex Microcontroller Software Interface Standard - a vendor-independent abstraction layer for ARM Cortex-M processors. Defines standard interfaces for peripherals, RTOS, DSP, etc.

**Why It's Important:**
Provides processor-level abstraction (more generic than vendor-specific), enables code portability across ARM Cortex-M devices, defines standard register definitions, and is ARM-maintained standard.

**How It's Used in CI/CD:**
CMSIS libraries are part of IDE-provided source libraries, copied into project. Provides core processor support and standard definitions. More generic than ST-specific HAL/LL.

**Example from Gene's Course:**
```c
// CMSIS-defined:
#include "stm32f4xx.h"  // Device header (CMSIS)
#include "core_cm4.h"    // Cortex-M4 core (CMSIS)

// CMSIS provides:
// - Processor registers (NVIC, SysTick, SCB, etc.)
// - Standard peripheral definitions
// - System initialization

// Project structure:
// Drivers/
//   ├── CMSIS/           (more generic, ARM standard)
//   ├── STM32F4xx_HAL/   (ST-specific)
//   └── ...
```

**Context:** "Driver Libraries: HAL (Hardware Abstraction Layer) style, LL (Low-Level) style, CMSIS (more generic, not just ST)."

**Related Concepts:**
- HAL
- LL Drivers
- ARM Cortex-M
- Vendor Independence

---

### ST-Link

**Definition:**
STMicroelectronics' debug probe and programmer for STM32 microcontrollers. Provides SWD/JTAG debugging interface and programming capability. Built into Nucleo boards, available as standalone adapter.

**Why It's Important:**
Enables debugging and flash programming, provides serial port (USB CDC), integrated on Nucleo boards for convenience, and each has unique serial number for multi-board setups.

**How It's Used in CI/CD:**
Built-in ST-Link on Nucleo provides debug and serial. Separate ST-Link adapter used for Blue Pill. ST-Link serial numbers passed to flash scripts to program correct board.

**Example from Gene's Course:**
```bash
# List connected ST-Links:
STM32_Programmer_CLI -l
# Output:
# SN: 066DFF535150898367092722 (Nucleo)

# Program specific board:
STM32_Programmer_CLI -c port=SWD sn=066DFF535150898367092722 -d image.elf 0x08000000

# Jenkins parameter:
# STLINK_SN: 066DFF535150898367092722

# Hardware setup:
# - Nucleo: Built-in ST-Link (USB provides serial + debug)
# - Blue Pill: External ST-Link adapter (for development, not CI/CD)
```

**Related Concepts:**
- STM32_Programmer_CLI
- ST-Link Serial Number
- Flash Programming
- Hardware Setup

---

### SMTP Server (Email)

**Definition:**
Simple Mail Transfer Protocol server - handles sending email messages. SMTP is standard protocol for email transmission between servers and from clients to servers.

**Why It's Important:**
Enables automated email notifications from CI/CD, provides build failure alerts to developers, industry-standard protocol, and widely supported.

**How It's Used in CI/CD:**
Jenkins configured with SMTP server settings (Gmail in course) to send email notifications. Extended Email Notification plugin handles email composition and sending. Triggered on unsuccessful builds.

**Example from Gene's Course:**
```
# Jenkins Global Configuration:
# Extended Email Notification:
#   SMTP server: smtp.gmail.com
#   Port: 465 (or 587)
#   Use SSL: Yes
#   Credentials: username/password
#
# Default email:
#   Recipients: geneschrader.cicdclass@gmail.com
#   Subject: $PROJECT_NAME - Build # $BUILD_NUMBER - $BUILD_STATUS!
#   Body: $DEFAULT_CONTENT

# Jenkinsfile trigger:
post {
    unsuccessful {
        emailext subject: '...', body: '...', to: '...'
    }
}
```

**Security Note:** Demo uses username/password. Production should use certificates or OAuth2.

**Related Concepts:**
- Email Notifications
- Less Secure App Access (Gmail)
- Certificate-Based Authentication
- Jenkins Plugins

---

### Generic Web Hook Trigger Plugin

**Definition:**
Jenkins plugin that creates HTTP endpoints for receiving web hooks. Supports token-based identification of which Jenkins project to trigger. Flexible parameter extraction from web hook payloads.

**Why It's Important:**
Enables Git hooks and other systems to trigger Jenkins builds, supports multiple projects with token-based routing, flexible enough for various web hook formats, and essential for automated build triggering.

**How It's Used in CI/CD:**
Plugin configured in Jenkins project with token "cicd_class_1". Git post-update hook sends HTTP request with this token. Plugin receives request and starts build.

**Example from Gene's Course:**
```groovy
# Jenkins Project Configuration:
# Build Triggers:
#   ☑ Generic Web Hook Trigger
#   Token: cicd_class_1

# Webhook URL format:
# http://jenkins-server/generic-webhook-trigger/invoke?token=<token>

# Git hook sends:
curl -u gene:password http://localhost:8080/generic-webhook-trigger/invoke?token=cicd_class_1

# Plugin:
# 1. Receives HTTP request
# 2. Checks token: "cicd_class_1"
# 3. Identifies Jenkins project
# 4. Starts build

# Multiple projects can use same plugin with different tokens
```

**Related Concepts:**
- Web Hooks
- Git Hooks
- Build Triggers
- Jenkins Plugins

---

### Extended Email Notification Plugin

**Definition:**
Jenkins plugin providing advanced email notification capabilities beyond Jenkins' basic email support. Supports templates, triggers, recipients lists, attachments, and flexible configuration.

**Why It's Important:**
Provides flexible email customization, supports various trigger conditions, enables HTML templates, allows per-project and global configuration, and is more capable than basic Jenkins email.

**How It's Used in CI/CD:**
Global Jenkins configuration defines SMTP settings and default email content. Pipeline uses emailext step in post section to send notifications on unsuccessful builds.

**Example from Gene's Course:**
```groovy
# Global Configuration:
# Extended Email Notification:
#   SMTP server: smtp.gmail.com
#   Default Recipients: geneschrader.cicdclass@gmail.com
#   Default Subject: $PROJECT_NAME - Build # $BUILD_NUMBER - $BUILD_STATUS!
#   Default Body: $DEFAULT_CONTENT

# Jenkinsfile usage:
post {
    unsuccessful {
        emailext subject: '$PROJECT_NAME - Build # $BUILD_NUMBER - $BUILD_STATUS!',
                 body: '$DEFAULT_CONTENT',
                 to: 'geneschrader.cicdclass@gmail.com'
    }
}

# Variables available:
# $PROJECT_NAME, $BUILD_NUMBER, $BUILD_STATUS, $DEFAULT_CONTENT, etc.
```

**Related Concepts:**
- Email Notifications
- SMTP Server
- Jenkins Plugins
- Post-Build Actions

---

## Summary: Production-Level Code Characteristics

After studying all these best practices, terminologies, and tools, what makes code "production-level" in the context of CI/CD?

### 1. **Reliability**
- Automated testing catches bugs early (HIL, unit, integration)
- Build ID verification ensures testing correct version
- Static code analysis finds issues compilers miss
- Clean builds ensure reproducibility
- Version control enables rollback if needed

### 2. **Maintainability**
- Source control provides history and collaboration
- Incremental commits create meaningful history
- Lean Jenkinsfile keeps automation simple
- Separation of concerns (app code, build files, settings)
- Documentation through commit messages and pipeline structure

### 3. **Testability in CI/CD**
- Automated build process (no manual steps)
- Automated flash programming
- Automated testing (HIL with stimulus-response)
- Test result reporting (JUnit XML)
- Quality gates prevent bad code from progressing

### 4. **Security**
- Key/certificate-based authentication (production)
- No secrets in source code
- Access control on repositories
- Secure communication (SSH, HTTPS, TLS)

### 5. **Team Collaboration**
- Multi-developer workflows supported
- Standardized practices (paths, workflow)
- CI provides rapid feedback to all developers
- Automated tests prevent integration problems
- Clear pipeline visualization

### 6. **Continuous Improvement**
- Pipeline reliability is paramount
- Start small: "Doing just a little is better than doing nothing"
- Incremental approach: build capabilities over time
- Learn from failures (Ring doorbell story)
- Adapt workflows to accommodate tool limitations

---

## Key Lessons for Improving Your Coding Level

1. **Automate Everything Possible**: Manual steps are error-prone and prevent continuous integration.

2. **Test Early and Often**: Static analysis + automated testing catch bugs when they're cheap to fix.

3. **Version Control is Fundamental**: Not just for code - makefiles, scripts, pipeline definitions all belong in Git.

4. **Build ID Verification is Critical**: Always verify you're testing what you think you're testing.

5. **Keep It Simple**: Lean Jenkinsfile, clear separation of concerns, minimal dependencies.

6. **Developer Workflow Matters**: Good workflow prevents CI/CD failures (build both Debug and Release!).

7. **Start Small, Iterate**: Even minimal automation provides value. Build up over time.

8. **Pipeline Reliability is Paramount**: Unreliable pipelines hurt more than they help.

9. **Security from the Start**: Demo shortcuts are fine for learning, but plan for production from day one.

10. **Tools Enable, Process Matters**: Having Jenkins doesn't give you CI/CD. The workflow and practices matter more.

---

**Course Philosophy Applied:**
"Doing just a little is better than doing nothing" - Gene Schrader

Start implementing these practices incrementally. Even adding .gitignore and using incremental commits improves your code quality. Add static analysis next. Then automated builds. Build your CI/CD capabilities over time, and your code will become more reliable, maintainable, and testable - the hallmarks of production-level code.

---

**End of Best Practices & Terminologies Guide**
