# Summarize introduction to embedded cicd course
_Exported on 10/23/2025 at 21:22:32 EDT from Cursor (1.7.54)_

---

**User**

Summarize @Embedded CICD with HIL Testing Lesson 1. Introduction to the Course.txt   compredhensively.



---

**Cursor**

# Comprehensive Summary: Embedded CI/CD with HIL Testing - Lesson 1

## Course Overview
**Instructor**: Gene Schrader  
**Focus**: Setting up Continuous Integration/Continuous Delivery (CI/CD) with hardware-in-the-loop (HIL) testing for embedded projects using STM32CubeIDE on a single Windows laptop.

## Target Audience
- Embedded developers wanting to bring CI/CD to their projects
- Job seekers encountering CI/CD in job descriptions
- Software engineering students wanting hands-on CI/CD experience

## Course Structure (Video Topics)
1. Introduction (this lesson)
2. Preparing STM32CubeIDE project for CI/CD (creating git repo)
3. Automating tasks via scripts (build, static analysis, flash programming, HIL testing, delivery)
4. Setting up Jenkins (creating pipeline to invoke automation scripts)
5. Creating git hooks to trigger Jenkins on code changes
6. Follow-on topics (improvements, STM32CubeIDE + git tips)

## Key CI/CD Concepts

### Continuous Delivery Definition
When new code is committed, an automated process generates a new software release (images) if all tests pass, making it available to testers/stakeholders.

### Main Benefits
- **Automation**: Saves work, prevents mistakes and oversights
- **Early feedback**: Catches bad code immediately before more code builds on top of it
- **Visibility**: Developers get notified of failures quickly while code is fresh in mind

### Challenges Addressed
- **GUI IDE automation**: STM32CubeIDE tasks (normally mouse-driven) must be scripted
- **HIL testing complexity**: May require simulating external hardware interfaces

### Philosophy
- Start small (even just static analysis is valuable)
- Add features incrementally over time
- Build testing culture into development workflow

## System Architecture

**All components run on single laptop** (in production would be distributed):

### Developer Side
- STM32CubeIDE with toolchain
- Git repo (working copy) within IDE project
- Product hardware + HIL simulation hardware
- Build outputs (flash images, map files) excluded from repo

### Git Server
- Central repository (official code copy)
- Configured with hooks to trigger Jenkins via web

### Build Server (Jenkins)
- Triggered by git server on code pushes
- Fetches code into Jenkins workspace
- Uses STM32CubeIDE toolchain (compiler/linker)
- Connects to product hardware for HIL testing
- On success: copies images to software release store
- On failure: sends email notification

### Workflow
1. Developer pushes code to git server (master branch)
2. Git hook triggers Jenkins build
3. Jenkins pulls code, builds, runs static analysis
4. Jenkins flashes firmware and runs HIL tests
5. If all passes: images copied to release directory
6. If fails: email sent to developer

## Hardware Requirements

### Product Hardware
- **Nucleo board** with Cortex-M4 (STM32F401RE)
- Portable to other STM32 boards with minor modifications

### HIL Simulation Hardware
- **STM32 Blue Pill board** for GPIO manipulation during testing
- ST-Link adapter (cheap Amazon version works)
- USB-to-serial cable adapter
- Jumpers for GPIO connections

**Note**: Course can be followed without hardware (builds/analysis work; flash/HIL tests will fail)

## Software Requirements
- STM32CubeIDE
- Git for Windows
- Jenkins
- Python 3.x (not 2.7)
- ST-Link utilities
- Serial terminal program
- Cppcheck (static analysis)

*Version numbers mentioned but noted as non-critical (except Python 3.x)*

## Prerequisites

**Not required but helpful**:
- **Git/source control**: Course explains all steps but parallel learning recommended
- **Windows batch scripting**: Used for automation (instructor keeps scripts simple)
- **STM32CubeIDE basics**: Minimal familiarity helpful but not critical
- **C programming**: Only small amounts reviewed

## Administrative Notes
- YouTube video updates difficult; corrections posted as pinned comments
- Repos available for both nucleo board and blue pill HIL hardware
- Feedback welcome via YouTube comments or Reddit DM

## Discussion Question
**Q**: Is CI/CD useful for single developer?  
**A**: Yes! Automation prevents forgetting steps and provides confidence. While more valuable with multiple developers (unexpected interactions), even solo developers benefit from automated validation and consistency.

## Instructor Background
Worked on similar CI/CD system with Jenkins and HIL testing starting in 2015 on large multi-developer project where high code change velocity made automated feedback critical.

---

**User**

Summarize @Embedded CICD with HIL Testing Lesson 2. Creating STM32CubeIDE Project and Git Repo.txt   compredhensively.


---

**Cursor**

# Comprehensive Summary: Embedded CI/CD with HIL Testing - Lesson 2

## Lesson Overview
**Topic**: Creating STM32CubeIDE Project and Git Repository  
**Focus**: Setting up source code management (SCM) as the foundational step for CI/CD implementation

## Lesson Scope (Dotted Oval from Architecture Diagram)
- IDE project creation
- Local git repository setup
- Remote repository configuration

## Background: STM32CubeIDE Architecture

### IDE System Components (Common/Shared)
1. **Eclipse IDE Base**: Core infrastructure
2. **MCU/Board Database**: Pin-out and hardware info for supported devices
3. **Pin & Hardware Configuration Editor**: Enable peripherals, assign pins, configure startup options
4. **Code Generator**: Auto-generates initialization code based on hardware configuration
5. **ST-Link Support**: Debugging interface integration

### IDE-Provided Libraries
- **Driver Libraries** (HAL/LL): Hardware interfacing (UART, I2C, etc.)
  - HAL (Hardware Abstraction Layer)
  - LL (Low-Level)
- **CMSIS**: Generic middleware (cross-vendor compatibility)
- **Middleware Libraries**: RTOS, file systems, TCP/IP stacks
- **Note**: IDE copies appropriate library versions into projects based on selected MCU

### MCU Toolchains
- Compiler, linker, standard C libraries
- Different versions for different MCU types
- Standard C functions: printf, math, string operations

### Project Contents (What Goes Where)

**Generated by IDE:**
- Pin/hardware configuration (.ioc file - I/O Configuration)
- Initialization code
- Makefiles
- Linker scripts
- Properties/settings files (often hidden, start with '.')

**User-Created:**
- Application code files
- Build/test scripts (for CI/CD pipeline)

**Generated by Toolchain:**
- Build output files (object files, map files, image files)

### Source Control Management Strategy

**Include in Git Repo:**
- Configuration files (.ioc)
- Generated initialization code
- Makefiles
- Linker scripts
- IDE-provided source libraries
- Properties/settings files
- Application code
- CI/CD scripts
- From Debug/Release folders: Makefiles and build-required files (e.g., objects.list)

**Exclude from Git Repo:**
- Build output files (object code, images) - regenerated on each build

## Step-by-Step Project Creation Process

### Approach: Incremental Commits
Build project in steps, committing changes at each stage to create development history

### Step 1: Create IDE Project & Initial Local Repo

**Commands:**
```bash
cd [project_folder]
git init                                    # Create local repo
git config user.name "Your Name"           # Set username (course-specific)
git config user.email "your@email.com"     # Set email (course-specific)
```

**Create .gitignore File:**
- Specifies file patterns to exclude from repo (wildcards supported)
- Prevents build outputs from appearing as untracked files
- Example entries:
  - `*.o`, `*.elf`, `*.map` (build outputs)
  - `*.su` (stack usage files)
  - Exception: `!objects.list` (required for builds despite being .list file)
  - `test-results*.xml` (HIL test outputs)

**Initial Commit:**
```bash
git add .                                  # Stage all untracked files
git commit -m "ide create project"        # Commit to repo
```

### Step 2: Integrate Application Code & CI/CD Scripts

**Actions:**
1. Create `App/` and `ci-cd-tools/` folders
2. Copy application files and scripts
3. Right-click App folder → Resource Configuration → Unexclude from build
4. Modify `Core/Src/main.c` to hook application:
   - Declare application function
   - Call `app_main()` in while loop

**Commit Changes:**
```bash
git add .
git commit -m "add application and cicd scripts"
```

### Step 3: Build Project & Update Makefiles

**Actions:**
1. Perform Debug and Release builds in IDE
2. IDE generates all necessary makefiles
3. Commit generated build files

**Commit:**
```bash
git add .
git commit -m "debug and release builds"
```

## Useful Git Commands

### Daily Workflow Commands
- `git status -u`: Show modified/created files (verify before commit)
- `git diff`: Compare workspace to previous version
- `git diff [commit1] [commit2]`: Compare between commits
- `git diff --name-only [commit1] [commit2]`: List changed files only
- `git log`: View commit history

### Command Examples from Lesson
```bash
# View specific file changes between commits
git diff [hash1] [hash2] Core/Src/main.c

# List changed files between commits  
git diff --name-only [hash1] [hash2]
```

**Note**: Commit hashes can be abbreviated (only unique prefix needed)

## Remote Repository (Git Server)

### Bare Repository Characteristics
- **Purpose**: Official code storage for sharing (like GitHub)
- **Type**: Bare repo (no working directory)
- **Cannot**: Be used for direct development/file modification
- **Workflow**: Make changes in local repo → push to bare repo
- **Access Method**: File URLs (simplified for single-laptop setup; normally SSH/HTTPS)

### Creating Bare Repo

**Commands:**
```bash
cd [repos_parent_directory]
mkdir ci-cd-class-1.git                    # Convention: .git suffix
cd ci-cd-class-1.git
git init --bare                            # Create bare repository
```

**URL Format:**
```
file:///C:/path/to/repos/ci-cd-class-1.git
```

### Pushing Local Repo to Remote

**Commands:**
```bash
# Define remote repo with alias
git remote add origin file:///C:/path/to/repos/ci-cd-class-1.git

# First-time push (sets default)
git push -u origin master

# Subsequent pushes
git push
```

## Special Case: Blue Pill Board (HIL Simulation Hardware)

### Configuration Challenges
- **Not ST board**: IDE lacks pre-configured support
- **Create project**: Select MCU type directly (not board)
- **More manual configuration required**

### Critical Configurations

1. **Enable Debug**: Essential for future flashing capability
2. **Configure UART**: For console serial port
3. **Clock Configuration**: 
   - Default: Very slow clock
   - Optimized: 72 MHz using 8 MHz crystal
   - Enable crystal in hardware configuration first
   - Adjust clock configuration tab (see lesson screenshot for parameters)

### Resources
Course references web guides for Blue Pill clock setup and configuration

## Design Philosophy

### Git Plugin Decision
- **Available**: STM32CubeIDE has Git GUI plugin
- **Chosen Approach**: Command-line Git
- **Rationale**:
  - Learn Git fundamentals
  - Course theme: Move from GUI to command line for automation
  - Command line provides more confidence/control
  - Author's experience: Often reverted from GUI to command line

## Discussion Question & Answer

**Q**: Downsides/advantages of creating repo in multiple steps vs. one commit?

**Downsides:**
- Slightly larger repo size (stores intermediate states)

**Advantages:**
- **Rollback capability**: Can revert specific commits if mistakes made
- **Development history**: Documents project evolution
- **Common practice note**: In real development, mistakes typically fixed with new commits rather than backing out commits (complications)

## Lesson Summary
1. Examined STM32CubeIDE project structure to determine SCM inclusion criteria
2. Created local git repository with incremental commits
3. Created remote bare repository for official code storage and Jenkins sharing
4. Pushed local repository contents to remote repository
5. Established foundation for CI/CD automation

## Key Takeaways
- Source code management is prerequisite for CI/CD (beneficial even without CI/CD)
- Not all project files belong in SCM (exclude regeneratable build outputs)
- Bare repositories enable code sharing between developers and CI/CD systems
- Incremental commits create valuable development history
- Command-line Git knowledge essential for automation scripting

```shellscript
cd [project_folder]
git init                                    # Create local repo
git config user.name "Your Name"           # Set username (course-specific)
git config user.email "your@email.com"     # Set email (course-specific)
```

```shellscript
git add .                                  # Stage all untracked files
git commit -m "ide create project"        # Commit to repo
```

```shellscript
git add .
git commit -m "add application and cicd scripts"
```

```shellscript
git add .
git commit -m "debug and release builds"
```

```shellscript
# View specific file changes between commits
git diff [hash1] [hash2] Core/Src/main.c

# List changed files between commits  
git diff --name-only [hash1] [hash2]
```

```shellscript
cd [repos_parent_directory]
mkdir ci-cd-class-1.git                    # Convention: .git suffix
cd ci-cd-class-1.git
git init --bare                            # Create bare repository
```

```plaintext
file:///C:/path/to/repos/ci-cd-class-1.git
```

```shellscript
# Define remote repo with alias
git remote add origin file:///C:/path/to/repos/ci-cd-class-1.git

# First-time push (sets default)
git push -u origin master

# Subsequent pushes
git push
```

---

**User**

Summarize @Embedded CICD with HIL Testing Lesson 3. Automation of STM32CubeIDE Tasks.txt  compredhensively.


---

**Cursor**

# Comprehensive Summary: Embedded CI/CD with HIL Testing - Lesson 3

## Lesson Overview
**Topic**: Automation of STM32CubeIDE Tasks  
**Focus**: Identifying which IDE tasks must be automated and creating scripts to execute them outside the GUI

## Task Categorization for CI/CD

### Manual Tasks (Red - Design Work)
**Not automated** - Performed in IDE as needed:
- Creating projects
- Configuring hardware
- Initial setup and design decisions

### Must-Automate Tasks (Green - Core CI/CD)
**Essential for CI/CD automation:**
- **Building code** → Generate flash images (Debug & Release configurations)
- **Programming flash memory** → Deploy images to hardware

### Optional/Not-Automated Tasks (Blue - Handled Manually)
**Course doesn't automate these:**

1. **Regenerating IDE-supplied code**
   - Triggered after hardware/software configuration changes
   - Usually done infrequently
   - IDE typically prompts for regeneration
   - **Not a big deal** - generate when making config changes

2. **Creating/updating makefiles**
   - Happens when adding/removing .c files or changing compiler settings
   - **Would be ideal to automate, but course doesn't handle this**
   - Requires developer workflow accommodation

## Recommended Developer Workflow

To accommodate manual makefile updates:

1. Make code changes in IDE project
2. Build and fix compiler errors
3. Test and debug code (iterate as needed)
4. **When ready to push to Git:**
   - **Critical step**: Do final IDE builds for BOTH Debug AND Release
   - Ensures makefiles are up-to-date
   - Even if no makefile changes, do this as safety measure
5. Commit file changes (code + makefiles)
6. Push to Git for CI/CD

**Assessment**: "Workable system even if not ideal"

## Build Automation

### Two Options for Automated Builds

#### Option 1: Headless STM32CubeIDE
- Run IDE via command-line parameters without GUI
- Example available in IDE installation files
- **Not chosen** - Author encountered issues, had doubts about reliability

#### Option 2: Direct Make (CHOSEN APPROACH)
- Use standard `make` tool directly
- IDE generates/maintains makefiles; scripts just invoke make

**Advantages:**
- Uses standard make tool
- No IDE dependence during build
- No "fighting with the IDE"
- Aligns with goal of moving away from IDE

**Disadvantages:**
- Depends on IDE to update makefiles (addressed by workflow above)

### Creating Build Script

**Information Source**: IDE Project Properties → Environment Variables

**Key Variables:**
- **CWD** (Current Working Directory): Where build must execute
- **PATH**: System PATH + toolchain additions from IDE

**Build Command:**
```bash
make -j4 all
```
- `-j4`: Run up to 4 parallel compile steps (dual-core with hyperthreading)
- `all`: Build target

### Simple Build Script Example

```batch
setlocal
set CWD=[path_from_IDE]
set PATH=[path_from_IDE]
cd %CWD%
make -j4 all
```

**Note**: Everything hard-coded; fine for testing, not production

### Jenkins Build Script Enhancements

**Located at**: `ci-cd-tools/build.bat`

**Improvements:**
- Passes parameters instead of hard-coding
- Accepts: configuration (Debug/Release), target (clean/all), paths, etc.
- **Version injection**: Creates `version.h` with build ID
  - Enables version verification via console command
  - Critical for confirming correct version being tested

**Version.h Generation:**
```batch
echo #define VERSION_ID "%BUILD_ID%" > version.h
```

## Flash Programming Automation

### Tool: STM32CubeProgrammer CLI

**GUI Version**: STM32CubeProgrammer (must download/install)  
**CLI Version**: `STM32_Programmer_CLI.exe` (included with GUI)

### Required Parameters

1. **Image file path**: `.elf` or `.hex` file to program
2. **Flash start address**: `0x08000000` (standard for all STM32 Cortex-M MCUs used by author)
   - Available in datasheet or linker script
3. **ST-Link serial number** (optional):
   - Required only if multiple ST-Links connected
   - Get list: `STM32_Programmer_CLI.exe -l`

### Simple Flash Script Example

```batch
set CLI=STM32_Programmer_CLI.exe
set STLINK=[serial_number]
set IMAGE=[path_to_image.elf]

%CLI% -c port=SWD sn=%STLINK% -d %IMAGE% 0x08000000 -hardRst
```

**Parameters:**
- `-c port=SWD sn=[serial]`: Connect via SWD with specific ST-Link
- `-d [image] [address]`: Download/program image at address
- `-hardRst`: Hardware reset after programming

### Jenkins Flash Script

**Located at**: `ci-cd-tools/flash.bat`  
**Enhancement**: Parameters passed instead of hard-coded values

## Critical Issue: Makefile Path Problem

### The Problem

**Generated makefile contained:**
```makefile
LINKER_SCRIPT = C:/Users/[username]/[full/path/to]/project/STM32F401RETX_FLASH.ld
```

**Why problematic:**
- Full path references IDE project folder
- Works for IDE builds
- **Fails for Jenkins builds** - can only use files from Git workspace
- Build cannot reference files outside Jenkins workspace

### Solution: Change to Relative Path

**Fix location**: Project Properties → C/C++ Build → Settings → MCU GCC Linker → General

**Change from:**
```
${workspace_loc:/${ProjName}/STM32F401RETX_FLASH.ld}
```

**Change to:**
```
../STM32F401RETX_FLASH.ld
```

**Result**: Path works from current working directory (build directory)

### Alternative Solution (from prompts)

Create pre-build script that edits makefile copy in Jenkins workspace to convert full paths to relative paths before building.

## Demo Scripts

### IDE-Specific Helper Scripts

Created for command-line builds from IDE project:

**buildi.bat** - Build from IDE project
```batch
buildi.bat [debug|release] [clean|all]
```

**flashi.bat** - Flash from IDE project  
```batch
flashi.bat [debug|release]
```

### Demo Sequence
```batch
buildi.bat debug clean     # Remove all object/image files
buildi.bat debug all       # Build everything
flashi.bat debug           # Program flash with debug build
```

## Discussion Questions & Answers

### Q1: Alternative fix for makefile path issue?

**A**: Create pre-build script executed by Jenkins that edits the makefile copy in workspace to change full paths to relative paths.

### Q2: Code changes cause Jenkins build to fail, but only for Release. What's the problem?

**A**: If changes affected makefiles, developer may have only done IDE build for Debug, not Release. Makefiles only updated for Debug folder.  
**Lesson**: Always do both Debug AND Release IDE builds before committing/pushing.

### Q3: Besides adding/removing source files, what changes require IDE builds to update makefiles?

**Examples:**
- **Changing compiler options** (e.g., `-D` preprocessor symbols)
- **Switching between HAL and LL libraries** (causes extensive makefile updates)
- Many other configuration changes

## Lesson Summary

### Accomplishments
1. **Identified automation requirements**: Building images and programming flash
2. **Created automation scripts**: 
   - Build script using standard `make`
   - Flash script using STM32CubeProgrammer CLI
3. **Resolved makefile path issue**: Changed to relative paths
4. **Established workflow**: Manual IDE builds before Git push to ensure makefiles current

### Key Takeaway
Standard tools (`make`, CLI programmer) enable automation without IDE GUI, though some manual steps (makefile updates) remain necessary with workable developer workflow.

### Next Steps
Continue preparation work for Jenkins pipeline in upcoming lessons (static analysis, HIL testing, etc.)

```shellscript
make -j4 all
```

```bat
setlocal
set CWD=[path_from_IDE]
set PATH=[path_from_IDE]
cd %CWD%
make -j4 all
```

```bat
echo #define VERSION_ID "%BUILD_ID%" > version.h
```

```bat
set CLI=STM32_Programmer_CLI.exe
set STLINK=[serial_number]
set IMAGE=[path_to_image.elf]

%CLI% -c port=SWD sn=%STLINK% -d %IMAGE% 0x08000000 -hardRst
```

```makefile
LINKER_SCRIPT = C:/Users/[username]/[full/path/to]/project/STM32F401RETX_FLASH.ld
```

```plaintext
${workspace_loc:/${ProjName}/STM32F401RETX_FLASH.ld}
```

```plaintext
../STM32F401RETX_FLASH.ld
```

```bat
buildi.bat [debug|release] [clean|all]
```

```bat
flashi.bat [debug|release]
```

```bat
buildi.bat debug clean     # Remove all object/image files
buildi.bat debug all       # Build everything
flashi.bat debug           # Program flash with debug build
```

---

**User**

Summarize @Embedded CICD with HIL Testing Lesson 4. Static Code Analysis and HIL Testing.txt   compredhensively. Include the related snippet code

The source code is in @base-hilt.py 

Don't make things up


---

**Cursor**

# Comprehensive Summary: Embedded CI/CD with HIL Testing - Lesson 4

## Lesson Overview
**Topics**: Static Code Analysis and Hardware-in-the-Loop (HIL) Testing  
**Focus**: Two key components for CI/CD pipeline preparation

## Part 1: Static Code Analysis

### Definition & Purpose
- **Static Analysis**: Examining code without running it (hence "static")
- **Tool-based**: Automated analysis (vs. manual code review by humans)
- **Goal**: Find potential bugs before code runs

### Benefits in CI/CD Pipeline
- Runs automatically on every code change
- No manual remembering required
- **Quick feedback**: Developer sees issues while code is fresh in mind
- Continuous quality enforcement

### Tool Selection: Cppcheck

**Why Cppcheck:**
- Free version available (also has licensed premium version)
- Easy to run from scripts
- Adequate for course needs

**Instructor's Experience:**
- Also used Coverity (licensed tool) in professional work
- "Did an incredible job at analyzing code and finding issues"

### Handling False Positives

**Suppression Comments**: Can add special comments to silence warnings on good code

**Example from Course:**
```c
// cppcheck-suppress comparePointers
// ... code that manipulates addresses/pointers in stack
```

**Context**: Fault handling code doing address manipulation for stack inspection - Cppcheck flagged it, but code was actually correct.

**Philosophy**: 
- Don't suppress unless necessary
- Sometimes better to refactor code to make Cppcheck happy

### Course-Specific Decisions

**Scope**: Only run Cppcheck on application code
- **Exclude**: IDE-provided code, third-party libraries
- **Rationale**: 
  - Not interested in trivial fixes to third-party code
  - Modifying third-party code creates software management burden
  - Only look for serious problems in third-party code if checking at all

**Pipeline Behavior on Failure:**
- Pipeline **continues** even if Cppcheck finds issues
- Build marked as **"unstable"** (not failed)
- Signals something not quite right while allowing downstream steps

## Part 2: Hardware-in-the-Loop (HIL) Testing

### Context: Types of Automated Testing

**Other testing approaches** (not covered in detail):
- **Unit tests**: Test individual functions/modules
- **Simulation-based tests**: 
  - Use test harnesses/mocks to simulate hardware
  - Run on build host (not target hardware)
  - Require product code to build/run on host
  - Often use `#ifdef` for special builds

**Course emphasis**: HIL testing, but use all testing types when possible

### Value Proposition

**Even minimal HIL testing is valuable:**
- Simply loading and running software
- Verifying basic startup via console
- **Prevents "dead on arrival" loads** - embarrassment of shipping 5 consecutive builds that won't boot

### Critical: Build ID Verification

**Real-World Story - Ring Doorbell:**
1. First product shipped right before Christmas
2. Immediate complaints about video quality
3. **Root cause**: Software changes made before ship, but testing setup had problem
4. **Actually tested old software** - new code never tested at all
5. Shipped untested software unknowingly
6. **Lesson**: "Gut wrenching" discovery
7. Lucky fix: Solved problem cloud-side
8. Could have killed the product otherwise

**Takeaway**: Always verify build ID in tests to ensure testing correct version

## HIL Testing Setup

### System Architecture Diagram

**Build Server (Laptop):**
- **Test Script**: `base-hilt.py` (Python)
- Can run from Jenkins pipeline or manually via command line
- Runs two **plink** programs (serial terminal from PuTTY suite)
- Communicates with both hardware boards

**Hardware Boards:**
1. **Nucleo Board** - Device Under Test (DUT) / Product
2. **Blue Pill Board** - HIL Simulation Hardware
   - Simulates external hardware
   - Controls/monitors GPIO

**Test Results:**
- **JUnit XML files** created by test script
- Standard format supported by Jenkins
- Jenkins displays test results in GUI

### Hardware Connections

**Blue Pill Board (HIL Simulation):**
- USB-to-Serial adapter cable (must be 3.3V compatible)
- ST-Link adapter (cheap Amazon version works)
- **Note**: ST-Link not needed for CI/CD (board not programmed in pipeline - just test equipment)

**Nucleo Board (DUT):**
- USB cable for ST-Link (built-in adapter)
- Provides serial link + debug interface for programming

**Inter-Board Connections:**
- GPIO: Port B Pin 9 (between boards)
- Ground tie (critical)

**Loopback Connection (on DUT):**
- GPIO loopback on Nucleo
- Enables some testing without Blue Pill board

## Application Software

### Design Philosophy

**Instructor's Candid Assessment:**
- "Not that happy about it"
- Written for easy understanding, not best practices
- Super loop with **extensive blocking** - "not good super loop design"
- Used STM32 HAL UART library for portability

**Why HAL:**
- Easier to port to other STM32 MCUs
- "Frankly it is hard to write a good console with this library and the code is a little ugly but it works"

**Interesting Note**: Same application runs on both product (Nucleo) and HIL simulation hardware (Blue Pill)
- "Sort of weird" but fine for demo purposes

### Console Commands

**Supported Commands:**
```
dc <port> <pin> <i|o> <mode>    # Configure pin
dr <port> <pin>                 # Read pin
dw <port> <pin> <value>         # Write pin
reset                            # Reset board
version                          # Get software version
```

### Application Code Structure

**Main Function** (`app/gpioapp/appmain.c`):

```c
// Simple super loop for command processor
while(1) {
    get_command_line();          // Get input from console
    parse_tokens();              // Parse into tokens
    process_command();           // Execute command
}
```

**GPIO Operations** (using HAL library):

```c
// Read pin
value = HAL_GPIO_ReadPin(...);

// Write pin  
HAL_GPIO_WritePin(...);
```

**Portability**: Already supports 4 boards; check `config.h` to port to other STM32 MCUs

## Python Test Script: `base-hilt.py`

### Technology Stack

**Language**: Python (instructor's favorite after C/C++)

**Key Python Modules:**
- **pexpect**: Stimulus-response testing (based on 1990s `expect` tool)
- **junit_xml**: Easy JUnit XML generation
- **plink**: Serial terminal from PuTTY suite

### Script Architecture

**Connection Management:**

```python:243:262:ci-cd-class-1-master/ci-cd-tools/base-hilt.py
class GpioDev:
    """ This class represents a device that support the GPIO CLI. """

    def __init__(self, name, serial_dev, baud_rate=115200):
        self.name = name
        _log.debug('[%s] Connecting to %s at %d', self.name, serial_dev,
                   baud_rate)
        self.console = pos.PopenSpawn('plink -serial %s -sercfg %d' %
                                      (serial_dev, baud_rate))
```

**Pattern Matching:**

```python:49:65:ci-cd-class-1-master/ci-cd-tools/base-hilt.py
def get_pattern_list(self, pattern_list):
    """ Wait for a series of patterns from the device, with an arbitrary amount
    of text allowed between the patterns.

    pattern_list - a list of text strings (can be regex) to wait for, in order.
    """

    rc = 0
    for pat in pattern_list:
        rc = self.console.expect([pat, pex.TIMEOUT, pex.EOF])
        if rc != 0:
            _log.debug('[%s] In get_pattern_list() failure for "%s" (rc=%d) for test "%s"',
                       self.name, pat, rc, g_test_name)
            return rc, pat
        _log.debug('[%s] Expecting "%s" got "%s%s"', self.name, pat,
                   self.console.before, self.console.after)
    return rc, None
```

### Test 1: Console Prompt Check

**Purpose**: Basic sanity - verify console responds

```python:163:177:ci-cd-class-1-master/ci-cd-tools/base-hilt.py
def test_console_prompt():
    """ Send a carriage return and wait for the console prompt. """

    passed = True
    start_test('console_prompt')
    g_dut.send_line('')
    rc = g_dut.get_prompt()
    if rc != 0:
        test_fail('Did not receive prompt rc=%d' % rc)
        passed = False
    else:
        test_pass()
    return passed
```

**Logic**: Send empty line, expect prompt. If no prompt, "something really wrong with the software"

### Test 2: Version Verification (CRITICAL)

**Purpose**: Ensure testing correct software version (Ring Doorbell lesson!)

```python:180:198:ci-cd-class-1-master/ci-cd-tools/base-hilt.py
def test_version(tver):
    """ Test to verify the software version is as expected.

    tver - the software version string.
    """

    passed = True
    start_test('version')
    g_dut.send_line('version')
    pat_list = ['Version="' + tver + '"', g_prompt]
    rc, failed_pat = g_dut.get_pattern_list(pat_list)
    if rc != 0:
        test_fail('Did not find pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        test_pass()
    return passed
```

**Flow:**
- Version string (`tver`) passed from Jenkins pipeline
- Send `version` command to board
- Expect `Version="<tver>"` + prompt
- **If mismatch**: Testing wrong version - test fails

### Test 3: GPIO Tests (Data-Driven)

**Design Pattern**: Repetitive tests → data-driven approach

**Test Structure** (each test is list of steps):

```python:245:253:ci-cd-class-1-master/ci-cd-tools/base-hilt.py
# Test D-1: DUT with pushpull output, SIM with no-pull input.
'd-1' : [
    [g_dut,    'dc b 9 o pushpull', 'OK'],
    [g_sim,    'dc b 9 i nopull',   'OK'],
    [g_dut,    'dw b 9 1',          'OK'],
    [g_sim,    'dr b 9',            '"1"'],
    [g_dut,    'dw b 9 0',          'OK'],
    [g_sim,    'dr b 9',            '"0"'],
    ],
```

**Step Format**: `[device, command, expected_response]`

**Example Test D-1 Logic:**
1. Configure DUT pin B9 as pushpull output → expect "OK"
2. Configure SIM pin B9 as no-pull input → expect "OK"
3. DUT writes 1 to pin B9 → expect "OK"
4. SIM reads pin B9 → expect "1"
5. DUT writes 0 to pin B9 → expect "OK"
6. SIM reads pin B9 → expect "0"

**Test Execution Loop:**

```python:383:406:ci-cd-class-1-master/ci-cd-tools/base-hilt.py
for name, test_steps in gpio_tests.items():
    # Check if test uses sim.
    skip_test = False
    for test_step in test_steps:
        if test_step[0] is None:
            skip_test = True
            break
    if skip_test:
        continue
    test_name = 'gpio-%s' % name
    start_test(test_name)
    step_idx = -1
    rc = 0
    for test_step in test_steps:
        step_idx += 1
        dev, send_msg, expect_msg = test_step
        dev.send_line(send_msg)
        rc, failed_pat = dev.get_pattern_list([expect_msg, g_prompt])
        if rc != 0:
            test_fail('At step idx %d did not find pattern "%s" rc=%d' %
                      (step_idx, failed_pat, rc))
            break
    if rc == 0:
        test_pass()
```

**Test Coverage**: 
- **D-tests** (d-1 to d-10): Use both DUT and SIM boards (inter-board communication)
- **S-tests** (s-1 to s-5): Single board loopback tests (DUT only, PC5↔PC6)

**Total**: 15 GPIO tests + 4 basic tests = **19 tests**

### JUnit XML Output

**Result Recording:**

```python:454:457:ci-cd-class-1-master/ci-cd-tools/base-hilt.py
test_suite = jux.TestSuite(args.jfile, g_test_junits)
print(jux.TestSuite.to_xml_string([test_suite]))
with open(args.jfile, 'w') as f:
    jux.TestSuite.to_file(f, [test_suite], prettyprint=True)
```

## Demo Session

### Successful Run
```bash
python3 base-hilt.py --log debug --tver "expected_version_string"
```

**Output:**
- Debug logging shows send/expect patterns
- Creates `test-results.xml`
- Summary: **19 tests, 0 disabled, 0 errors, 0 failures** ✓

### Failure Demonstration

**Induced Failures:**
1. Removed GPIO jumper (causes GPIO tests to fail)
2. Added "xxx" to version string (forces version mismatch)

**Result:**
- **5 failures** detected
- Version test failed: "didn't get the pattern it expected"
- 4 GPIO tests failed: timeout waiting for expected values
- Shows tests properly detect real issues

## Discussion Questions & Answers

### Q1: Can you do HIL testing without serial console?

**A**: Absolutely. This course is simplified/idealized case.

**Alternative approaches:**
- More extensive HIL simulation hardware to simulate sensors, user interfaces
- Network-based testing (IoT products) - simulate cloud server
- All controlled by test script

**Key point**: Can start HIL testing very simply in CI/CD pipeline

### Q2: How to control sensor readings (pressure, temperature)?

**A**: Difficult - depends on technical details.

**Possibilities:**
1. **Ideal**: Electrically simulate sensor (e.g., varying voltages)
2. **Fallback**: Test console commands on DUT to override sensor readings at low software level
   - Not pure black-box testing
   - Not complete coverage
   - **Still adds value**
   - Perhaps only in debug builds

**Instructor's view**: "Tests don't have to be 100% black box to be useful"

**Reality**: "Legitimate reason why people often stay away from hardware-in-the-loop because it has some difficulties"

### Q3: Static analysis finds problems but code compiles and tests pass - deliver the build?

**Pros (deliver anyway):**
- Static analysis issues often minor coding problems
- If someone waiting for bug fix, deliver ASAP
- Don't delay delivery for trivial issues

**Cons (don't deliver):**
- Could be subtle problem not caught by automated tests
- Users could waste time on problematic build
- **Strictness**: Don't want habit of ignoring static analysis reports
- Enforce quality standards - fix issues before delivery

### Q4: How to run HIL tests from different server (not build server)?

**A**: Many approaches possible.

**Suggested architecture:**
- Build server runs HIL script that **communicates with** test server
- Test server = separate physical machine (maybe in lab)
- **REST API**: Test server provides simple REST interface
  - Initiate test
  - Get results
- Build server uses simple tool like **curl** for communication

## Lesson Summary

### Key Accomplishments

1. **Static Code Analysis**: 
   - Simple method for automated code quality checks
   - Good CI/CD pipeline component
   - Cppcheck chosen for free availability and ease of use

2. **HIL Testing Example**:
   - Python script-driven testing from build machine
   - Simple but demonstrates principles

3. **HIL Simulation Hardware**:
   - Blue Pill board controls/monitors GPIO on DUT
   - Enables realistic testing without manual intervention

4. **JUnit XML Format**:
   - Standard test result recording
   - Jenkins can parse and display results
   - Python module makes generation easy

### Philosophy
- Even simple/imperfect HIL testing adds significant value
- Don't let "perfect" be enemy of "good enough"
- Start small, build incrementally
- Verify build ID to avoid shipping untested code

```c
// cppcheck-suppress comparePointers
// ... code that manipulates addresses/pointers in stack
```

```plaintext
dc <port> <pin> <i|o> <mode>    # Configure pin
dr <port> <pin>                 # Read pin
dw <port> <pin> <value>         # Write pin
reset                            # Reset board
version                          # Get software version
```

```c
// Simple super loop for command processor
while(1) {
    get_command_line();          // Get input from console
    parse_tokens();              // Parse into tokens
    process_command();           // Execute command
}
```

```c
// Read pin
value = HAL_GPIO_ReadPin(...);

// Write pin  
HAL_GPIO_WritePin(...);
```

```plaintext
class GpioDev:
    """ This class represents a device that support the GPIO CLI. """

    def __init__(self, name, serial_dev, baud_rate=115200):
        self.name = name
        _log.debug('[%s] Connecting to %s at %d', self.name, serial_dev,
                   baud_rate)
        self.console = pos.PopenSpawn('plink -serial %s -sercfg %d' %
                                      (serial_dev, baud_rate))
```

```plaintext
def get_pattern_list(self, pattern_list):
    """ Wait for a series of patterns from the device, with an arbitrary amount
    of text allowed between the patterns.

    pattern_list - a list of text strings (can be regex) to wait for, in order.
    """

    rc = 0
    for pat in pattern_list:
        rc = self.console.expect([pat, pex.TIMEOUT, pex.EOF])
        if rc != 0:
            _log.debug('[%s] In get_pattern_list() failure for "%s" (rc=%d) for test "%s"',
                       self.name, pat, rc, g_test_name)
            return rc, pat
        _log.debug('[%s] Expecting "%s" got "%s%s"', self.name, pat,
                   self.console.before, self.console.after)
    return rc, None
```

```plaintext
def test_console_prompt():
    """ Send a carriage return and wait for the console prompt. """

    passed = True
    start_test('console_prompt')
    g_dut.send_line('')
    rc = g_dut.get_prompt()
    if rc != 0:
        test_fail('Did not receive prompt rc=%d' % rc)
        passed = False
    else:
        test_pass()
    return passed
```

```plaintext
def test_version(tver):
    """ Test to verify the software version is as expected.

    tver - the software version string.
    """

    passed = True
    start_test('version')
    g_dut.send_line('version')
    pat_list = ['Version="' + tver + '"', g_prompt]
    rc, failed_pat = g_dut.get_pattern_list(pat_list)
    if rc != 0:
        test_fail('Did not find pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        test_pass()
    return passed
```

```plaintext
# Test D-1: DUT with pushpull output, SIM with no-pull input.
'd-1' : [
    [g_dut,    'dc b 9 o pushpull', 'OK'],
    [g_sim,    'dc b 9 i nopull',   'OK'],
    [g_dut,    'dw b 9 1',          'OK'],
    [g_sim,    'dr b 9',            '"1"'],
    [g_dut,    'dw b 9 0',          'OK'],
    [g_sim,    'dr b 9',            '"0"'],
    ],
```

```plaintext
for name, test_steps in gpio_tests.items():
    # Check if test uses sim.
    skip_test = False
    for test_step in test_steps:
        if test_step[0] is None:
            skip_test = True
            break
    if skip_test:
        continue
    test_name = 'gpio-%s' % name
    start_test(test_name)
    step_idx = -1
    rc = 0
    for test_step in test_steps:
        step_idx += 1
        dev, send_msg, expect_msg = test_step
        dev.send_line(send_msg)
        rc, failed_pat = dev.get_pattern_list([expect_msg, g_prompt])
        if rc != 0:
            test_fail('At step idx %d did not find pattern "%s" rc=%d' %
                      (step_idx, failed_pat, rc))
            break
    if rc == 0:
        test_pass()
```

```plaintext
test_suite = jux.TestSuite(args.jfile, g_test_junits)
print(jux.TestSuite.to_xml_string([test_suite]))
with open(args.jfile, 'w') as f:
    jux.TestSuite.to_file(f, [test_suite], prettyprint=True)
```

```shellscript
python3 base-hilt.py --log debug --tver "expected_version_string"
```

---

**User**

Summarize @Embedded CICD with HIL Testing Lesson 5. Setting up Jenkins.txt   compredhensively. include the related snippet code

don't make things up

---

**Cursor**

# Comprehensive Summary: Embedded CI/CD with HIL Testing - Lesson 5

## Lesson Overview
**Topic**: Setting up Jenkins  
**Focus**: Pull together all preparation work into a functional Jenkins CI/CD pipeline

## Introduction to Jenkins

### Instructor's Perspective
"I like to look at things in simple ways and to me I've used Jenkins as simply a **sophisticated script runner and data store specialized for DevOps**"

### Plugin Architecture
**Pros:**
- Third parties easily add new features
- Makes Jenkins powerful

**Cons:**
- More poking around to figure out usage
- Documentation sometimes inconsistent
- Popular plugins have good web discussion

### Project Styles

**Freestyle Projects:**
- Works fine (instructor's past experience)
- Some pipeline support but "have to do a lot yourself"

**Pipeline Projects (CHOSEN):**
- More complete pipeline support
- Modern approach
- Used in this course

### Pipeline Types

**Scripted Pipelines:**
- Original approach
- Powerful but complex scripting language

**Declarative Pipelines (CHOSEN):**
- Simpler syntax
- More popular
- Can embed advanced script pieces when needed
- "Best of both worlds"

**Identification**: Declarative starts with `pipeline`, scripted starts with `node`

### Jenkinsfile Storage Options

**Option 1**: Store on Jenkins server in application  
**Option 2 (CHOSEN)**: Store with product source code in Git repo

**Advantages of Option 2:**
- CI/CD tools under same source control as software
- Coordinated changes simplified
- History maintained together

### Best Practice Philosophy
**"Lean Jenkinsfile":**
- Doesn't contain much logic
- Calls lower-level scripts to do work
- More independent of Jenkins
- More logic in own scripts
- Learn less Jenkins script language

## Installation Hints (Windows 10)

### Java JDK Issues

**Initial Problem:**
- Plugin installation failures
- Certificate/security errors
- Suspicious of Java JDK selection
- Confusion about OpenJDK variants

**Solution:**
- Started over with known-working JDK from web recommendation
- Did **not** add to PATH
- **Required**: Set `JAVA_HOME` environment variable (how Jenkins finds JDK)

### Windows User/Service Configuration

**Challenge**: Jenkins runs as Windows service, needs user to start it

**Options:**
1. Create new user
2. Use own Windows login (instructor's choice)

**Required**: Modify Windows security permissions

**Steps:**
1. Run "Local Security Policy" app (Windows)
2. Navigate: Add user privilege to start Jenkins service
3. Add your username to allowed list

**Screenshot note**: Instructor had to add "gene" to privilege list

### Jenkins Interface

**Access URL:**
```
http://localhost:8080
```
- Default port: 8080
- `localhost` = on your own laptop

### Required Plugins

**List installed** (some may be default, verify before starting):
- Extended Email Notification
- Generic Webhook Trigger Plugin
- JUnit Plugin
- (Others mentioned in lesson)

## Email Configuration (Global Level)

### Navigation
Dashboard → Manage Jenkins → Configure System → Extended Email Notification

### SMTP Configuration

**Gmail SMTP Server**: Configured in Extended Email section (appears after plugin install)

**Authentication:**
- Username and password (not very secure)
- **Security Requirement**: Enable "Less secure app access" in Google Account → Security
- **Warning**: Google may disallow this in future
- **Better approach**: Use certificates (instructor was "too lazy")

**Default Recipients:**
```
[instructor's course email]
```

**Email Template Variables:**
```
$PROJECT_NAME        # Jenkins global variable
$BUILD_STATUS        # Jenkins global variable
# ... other documented Jenkins variables
```

**Purpose**: Provide useful information in email subject/body

## Jenkins Pipeline Configuration

### Build History Display

**Visual representation:**
- Each row = one build
- Columns = stages in build
- Stages shown:
  1. Checkout (from SCM)
  2. Build
  3. Static code analysis
  4. Program flash (Debug)
  5. Test Debug (HIL)
  6. Program flash (Release)
  7. Test Release (HIL)
  8. Post-build actions

**Color coding:**
- Green = success
- Yellow = unstable
- Red = failed
- Post-build actions execute even if build fails

### Stage Failure Strategy

**Design decision**: When stage fails, should pipeline continue?

**Course implementation:**
- Test failures → **STOP** (don't continue to next stage)
- Static analysis failures → **CONTINUE** (mark unstable)

### Parameterized Build

**Parameters defined** (for HIL testing flexibility):

```
COM_PORT_DUT          # COM port for Device Under Test
COM_PORT_SIM          # COM port for Simulation hardware
STLINK_SERIAL         # ST-Link serial number for programming
```

**Each parameter has:**
- Name
- Default value (matches instructor's setup)
- Description

### Build Triggers

**Manual build**: Always available via button

**Automated build**: Generic Webhook Trigger

**Key configuration:**
```
Token: [specific_token_string]
```
**Purpose**: Git hook includes this token so Jenkins knows which job to build

### Pipeline Script Source

**Option 1**: Put script directly in Jenkins form  
**Option 2 (CHOSEN)**: Script in Source Code Management

**Configuration:**
```
Pipeline script from SCM
SCM: Git
Repository URL: file:///C:/path/to/repos/ci-cd-class-1.git
Credentials: [none needed - file repo]
Script Path: ci-cd-tools/Jenkinsfile
```

## Jenkinsfile Structure

### Environment Section

```groovy
pipeline {
    environment {
        TOOL_DIR = "${WORKSPACE}/ci-cd-tools"
    }
```

**Variables:**
- `TOOL_DIR`: Location of build tools
- `WORKSPACE`: Jenkins-defined variable (top directory where code fetched)

### Stage 1: Build

```groovy
stage('Build') {
    steps {
        // Debug build
        bat "\"${TOOL_DIR}/build.bat\" Debug all \"${WORKSPACE}\""
        
        // Release build
        bat "\"${TOOL_DIR}/build.bat\" Release all \"${WORKSPACE}\""
    }
}
```

**Step type**: `bat` = Windows batch file  
**Failure behavior**: If bat script fails → stage fails → entire build fails (default)

### Stage 2: Static Code Analysis

```groovy
stage('Static Code Analysis') {
    steps {
        catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
            bat "\"${TOOL_DIR}/static-analysis.bat\""
        }
    }
}
```

**Key feature**: `catchError`
- If script fails → mark stage as FAILED
- But overall build marked as **UNSTABLE** (not failed)
- Pipeline continues

### Stage 3: Program Flash & Test Debug

```groovy
stage('Program Flash Debug') {
    steps {
        bat "\"${TOOL_DIR}/flash.bat\" Debug \"${WORKSPACE}\""
    }
}

stage('Test Debug') {
    steps {
        bat "python3 \"${TOOL_DIR}/base-hilt.py\" --jfile test-results.xml --tver ${BUILD_ID} --dut-serial ${COM_PORT_DUT} --sim-serial ${COM_PORT_SIM}"
        
        junit 'test-results.xml'
        
        bat "\"${TOOL_DIR}/check-test-results.bat\" test-results.xml"
    }
}
```

**Steps breakdown:**
1. **Run test script**: Passes console ports, version ID, output file
2. **junit step**: From JUnit XML plugin - parses results for display
3. **check-test-results.bat**: Validates XML; fails if any test failures detected

**Critical note**: Neither Python script nor `junit` step fail on test failures  
**Solution**: `check-test-results.bat` explicitly checks and fails to stop pipeline

### Stage 4: Program Flash & Test Release

```groovy
stage('Program Flash Release') {
    steps {
        bat "\"${TOOL_DIR}/flash.bat\" Release \"${WORKSPACE}\""
    }
}

stage('Test Release') {
    steps {
        // Same as Test Debug but for Release build
        bat "python3 \"${TOOL_DIR}/base-hilt.py\" ..."
        junit 'test-results.xml'
        bat "\"${TOOL_DIR}/check-test-results.bat\" test-results.xml"
    }
}
```

### Post-Build Actions

```groovy
post {
    success {
        bat "\"${TOOL_DIR}/deliver.bat\" \"${WORKSPACE}\""
    }
    
    unsuccessful {
        emailext (
            subject: "${env.JOB_NAME} - Build # ${env.BUILD_NUMBER} - ${currentBuild.result}",
            body: "Check console output at ${env.BUILD_URL} to view the results.",
            to: "[email_address]"
        )
    }
}
```

**Success action**: `deliver.bat`
- Packages image files + other artifacts
- Copies to defined delivery location on C: drive

**Failure action**: Send email notification
- Uses Extended Email plugin (`emailext`)
- Jenkins variables provide build info
- Overrides global email defaults

## Git Web Hooks

### Hook Concepts

**Definition**: Scripts that execute when specific Git actions occur

**Types:**
- Client-side hooks
- Server-side hooks (USED IN COURSE)

**Hook location:**
- **Server (bare) repo**: `hooks/` folder
- Client repo: `.git/hooks/` folder

**"Magic names"**: Create script with correct filename → Git automatically calls it

### Post-Update Hook (Server-Side)

**When it runs**: After new software pushed to repo

**File location:**
```
[repos_directory]/ci-cd-class-1.git/hooks/post-update
```

**Magic filename**: `post-update` (no extension, no `.sample` suffix)

**Hook script contents:**

```bash
#!/bin/sh

curl http://jenkins:[password]@localhost:8080/generic-webhook-trigger/invoke?token=[TOKEN_STRING]
```

**Script breakdown:**
- `#!/bin/sh`: Required shebang (Linux-style, works on Windows somehow)
- `curl`: HTTP transaction tool
- `jenkins:[password]`: Authentication (not secure, but acceptable for local demo)
- `localhost:8080`: Jenkins server
- `/generic-webhook-trigger/invoke`: Plugin-required URL endpoint
- `?token=[TOKEN]`: Matches token configured in Jenkins pipeline

**Note**: Sample hooks in `hooks/` folder have `.sample` extension → Git ignores them

## Demo: Static Analysis Detection

### Code Change

**Modified file**: `main.c`

```c
void dummy(void) {
    int *ptr;
    ptr = NULL;
    *ptr = 5;  // Null pointer dereference
}
```

**Compiler**: Did not detect issue  
**Static analysis**: Will catch it

### Git Commands

```bash
git status
# Shows: main.c modified

git diff
# Shows: added dummy() function

git add main.c

git status
# Shows: main.c ready to commit

git commit -m "bad dummy function"

git push
# Triggers Git hook → Triggers Jenkins
```

### Jenkins Build Result

**Immediate response**: New build appeared in Jenkins (triggered by hook)

**Stages executed:**
1. ✓ Checkout from Git
2. ✓ Build (compiler didn't catch issue)
3. ⚠ **Static Code Analysis - FAILED**
4. ✓ Program Flash Debug
5. ✓ Test Debug (all tests passed)
6. ✓ Program Flash Release  
7. ✓ Test Release (all tests passed)
8. ✓ Post-build actions

**Overall status**: **UNSTABLE** (yellow) - build succeeded but has warnings

### Console Output Analysis

**Navigation**: Build page → Console Output

**Static analysis section:**
```
[static-analysis.bat executing...]
[cppcheck running...]

[main.c:X]: (error) Null pointer dereference
    ptr is initialized to NULL [line Y]
    ptr is dereferenced [line Z]
```

**Lesson**: "Compiler didn't notice this but static code checker did - good example of how it can help find problems"

### Test Results Display

**Navigation**: Build page → Test Results

**Display shows:**
- All tests listed with status
- Duration (0 for all - needs more work in test script)
- **"Fixed" status**: Tests that failed in previous run but pass now (previous demo had removed GPIO jumper)

**Test summary**: 19 tests, all passed

### Email Notification

**Gmail account received:**
```
Build #4 has result: UNSTABLE
```

**Purpose**: Long builds → developer gets notification to check Jenkins without constant monitoring

## Discussion Questions & Answers

### Q1: Advantage of doing flash programming & testing AFTER software packaging?

**A**: "Ideally the software should be installed into flash memory or the product exactly how it would be done by a normal user"

**Course context**: Simple project has no official installation procedure, so didn't matter

### Q2: Why does Jenkins wait a few seconds after getting Git trigger?

**A**: "Simply a guess, but perhaps if the project uses two Git repos and you want to give the person time to push both repos before the build starts"

**Configuration**: Wait time is tunable (e.g., can set to 30 seconds)

## Lesson Summary

### Accomplishments

1. **Jenkins Introduction**: Script runner/data store for DevOps
2. **Installation guidance**: Java JDK, Windows service/user configuration
3. **Email setup**: Global SMTP configuration for build notifications
4. **Pipeline creation**: 
   - Parameterized builds
   - Webhook triggers
   - Declarative Jenkinsfile from Git repo
5. **Jenkinsfile structure**: 
   - Build stages
   - Static analysis with unstable marking
   - HIL testing with result validation
   - Post-build delivery and notifications
6. **Git hooks**: Automated pipeline triggering on code push
7. **Working demo**: End-to-end automation from code push to build notification

### Key Architectural Decisions

- **Declarative pipeline**: Simpler than scripted
- **Lean Jenkinsfile**: Calls external scripts for work
- **Jenkinsfile in Git**: Version controlled with code
- **Continue on static analysis failure**: Mark unstable, don't stop
- **Stop on test failure**: Don't continue to next stage
- **Email on failure**: Notify developer of issues

### Pipeline Flow Summary

```
1. Git push → Hook triggers Jenkins
2. Jenkins fetches code from Git
3. Build Debug & Release (fail → stop)
4. Static analysis (fail → unstable, continue)
5. Flash & Test Debug (fail → stop)
6. Flash & Test Release (fail → stop)
7. Success → deliver.bat packages artifacts
8. Failure → email notification sent
```

```plaintext
http://localhost:8080
```

```plaintext
[instructor's course email]
```

```plaintext
$PROJECT_NAME        # Jenkins global variable
$BUILD_STATUS        # Jenkins global variable
# ... other documented Jenkins variables
```

```plaintext
COM_PORT_DUT          # COM port for Device Under Test
COM_PORT_SIM          # COM port for Simulation hardware
STLINK_SERIAL         # ST-Link serial number for programming
```

```plaintext
Token: [specific_token_string]
```

```plaintext
Pipeline script from SCM
SCM: Git
Repository URL: file:///C:/path/to/repos/ci-cd-class-1.git
Credentials: [none needed - file repo]
Script Path: ci-cd-tools/Jenkinsfile
```

```groovy
pipeline {
    environment {
        TOOL_DIR = "${WORKSPACE}/ci-cd-tools"
    }
```

```groovy
stage('Build') {
    steps {
        // Debug build
        bat "\"${TOOL_DIR}/build.bat\" Debug all \"${WORKSPACE}\""
        
        // Release build
        bat "\"${TOOL_DIR}/build.bat\" Release all \"${WORKSPACE}\""
    }
}
```

```groovy
stage('Static Code Analysis') {
    steps {
        catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
            bat "\"${TOOL_DIR}/static-analysis.bat\""
        }
    }
}
```

```groovy
stage('Program Flash Debug') {
    steps {
        bat "\"${TOOL_DIR}/flash.bat\" Debug \"${WORKSPACE}\""
    }
}

stage('Test Debug') {
    steps {
        bat "python3 \"${TOOL_DIR}/base-hilt.py\" --jfile test-results.xml --tver ${BUILD_ID} --dut-serial ${COM_PORT_DUT} --sim-serial ${COM_PORT_SIM}"
        
        junit 'test-results.xml'
        
        bat "\"${TOOL_DIR}/check-test-results.bat\" test-results.xml"
    }
}
```

```groovy
stage('Program Flash Release') {
    steps {
        bat "\"${TOOL_DIR}/flash.bat\" Release \"${WORKSPACE}\""
    }
}

stage('Test Release') {
    steps {
        // Same as Test Debug but for Release build
        bat "python3 \"${TOOL_DIR}/base-hilt.py\" ..."
        junit 'test-results.xml'
        bat "\"${TOOL_DIR}/check-test-results.bat\" test-results.xml"
    }
}
```

```groovy
post {
    success {
        bat "\"${TOOL_DIR}/deliver.bat\" \"${WORKSPACE}\""
    }
    
    unsuccessful {
        emailext (
            subject: "${env.JOB_NAME} - Build # ${env.BUILD_NUMBER} - ${currentBuild.result}",
            body: "Check console output at ${env.BUILD_URL} to view the results.",
            to: "[email_address]"
        )
    }
}
```

```plaintext
[repos_directory]/ci-cd-class-1.git/hooks/post-update
```

```shellscript
#!/bin/sh

curl http://jenkins:[password]@localhost:8080/generic-webhook-trigger/invoke?token=[TOKEN_STRING]
```

```c
void dummy(void) {
    int *ptr;
    ptr = NULL;
    *ptr = 5;  // Null pointer dereference
}
```

```shellscript
git status
# Shows: main.c modified

git diff
# Shows: added dummy() function

git add main.c

git status
# Shows: main.c ready to commit

git commit -m "bad dummy function"

git push
# Triggers Git hook → Triggers Jenkins
```

```plaintext
[static-analysis.bat executing...]
[cppcheck running...]

[main.c:X]: (error) Null pointer dereference
    ptr is initialized to NULL [line Y]
    ptr is dereferenced [line Z]
```

```plaintext
Build #4 has result: UNSTABLE
```

```plaintext
1. Git push → Hook triggers Jenkins
2. Jenkins fetches code from Git
3. Build Debug & Release (fail → stop)
4. Static analysis (fail → unstable, continue)
5. Flash & Test Debug (fail → stop)
6. Flash & Test Release (fail → stop)
7. Success → deliver.bat packages artifacts
8. Failure → email notification sent
```

