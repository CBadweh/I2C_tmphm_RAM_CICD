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
