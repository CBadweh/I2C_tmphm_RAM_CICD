# Embedded CI/CD with Hardware-in-the-Loop Testing - Course Syllabus

**Instructor:** Gene Schrader
**Platform:** Windows 10
**IDE:** STM32CubeIDE (Eclipse-based)
**Duration:** 6 Lessons
**Format:** Hands-on demonstration with real hardware

---

## Course Overview

This course demonstrates how to build a complete Continuous Integration/Continuous Delivery (CI/CD) system for embedded systems development using a GUI-based IDE. The system includes automated building, static code analysis, flash programming, and hardware-in-the-loop (HIL) testing - all running on a single Windows laptop.

### Key Learning Outcome
Students will build an end-to-end automated embedded software delivery pipeline that:
- Automatically builds firmware on code commits
- Performs static code analysis
- Programs actual hardware
- Runs automated hardware tests
- Delivers validated releases
- Provides immediate feedback via email notifications

---

## Course Objectives

### Primary Objectives
1. **Implement CI/CD for embedded systems** using GUI-based development tools
2. **Automate repetitive tasks** (building, flash programming, testing)
3. **Design and implement HIL testing** for embedded firmware validation
4. **Integrate static code analysis** into the development workflow
5. **Configure Jenkins pipelines** for embedded systems
6. **Understand Git workflows** for embedded projects with IDE-generated files

### Secondary Objectives
7. Build automated testing mindset into development culture
8. Provide early, visible feedback on code quality
9. Prevent "dead on arrival" firmware loads
10. Scale CI/CD approach to multi-developer teams

---

## Hardware Requirements

### Product Hardware (Device Under Test)
- **Board:** STMicroelectronics Nucleo-F401RE
- **MCU:** STM32F401RE (ARM Cortex-M4, 84 MHz)
- **Connection:** Built-in ST-Link V2-1 (USB)
- **Purpose:** Represents the actual product being developed

### HIL Simulation Hardware
- **Board:** STM32 Blue Pill
- **MCU:** STM32F103C8T6 (ARM Cortex-M3, 72 MHz)
- **Connections:**
  - USB-to-serial adapter (3.3V) for console communication
  - ST-Link V2 adapter for programming/debugging (development only)
- **Purpose:** Simulates external hardware/sensors for automated testing

### Interconnections
- **GPIO Link:** Port B Pin 9 (Blue Pill ↔ Nucleo)
- **Ground:** Common ground between boards
- **USB Cables:** For both boards to connect to laptop
- **Optional:** USB extension cables for easier cable management

---

## Software & Tools Stack

### Development Tools
| Tool | Version/Notes | Purpose |
|------|---------------|---------|
| **STM32CubeIDE** | Latest | Primary embedded IDE (Eclipse-based) |
| **Git** | 2.x+ | Source code management |
| **Jenkins** | Latest LTS | Build automation server |
| **Python** | 3.x (NOT 2.7) | Test script language |
| **Java JDK** | Specific compatible version | Jenkins requirement (JAVA_HOME must be set) |

### Command-Line Tools
| Tool | Purpose |
|------|---------|
| **STM32_Programmer_CLI** | Flash memory programming from command line |
| **cpp-check** | Static code analysis (free version) |
| **make** | Build automation (comes with STM32CubeIDE toolchain) |
| **curl** | HTTP requests for Git web hooks |

### Python Packages
| Package | Purpose |
|---------|---------|
| **p-expect** | Stimulus-response testing framework |
| **junit_xml** | Generate JUnit XML test reports for Jenkins |
| **p-link** | Serial terminal (from PuTTY suite) |

### Jenkins Plugins
| Plugin | Purpose |
|--------|---------|
| **Generic Web Hook Trigger** | Trigger builds from Git post-update hooks |
| **Extended Email Notification** | Send email alerts on build failures |
| **JUnit** | Display test results in Jenkins UI |
| **Git Plugin** | SCM integration (may be installed by default) |

---

## Project Architecture

### System Components

```
┌─────────────────────┐
│   Git Server        │ (Bare repository - official code storage)
│   (File-based)      │
└──────────┬──────────┘
           │
    ┌──────┴───────┐
    │              │
    ↓              ↓
┌─────────────┐  ┌──────────────┐
│  Developer  │  │   Jenkins    │
│ Environment │  │ Build Server │
│             │  │              │
│ - IDE       │  │ - Workspace  │
│ - Local Git │  │ - Toolchain  │
│ - Hardware  │  │ - Hardware   │
└─────────────┘  └──────┬───────┘
                        │
                        ↓
                 ┌──────────────┐
                 │   Software   │
                 │Release Store │
                 └──────────────┘
```

### CI/CD Pipeline Flow

1. **Developer commits code** → Local Git repo
2. **Push to Git server** → Triggers post-update hook
3. **Git hook calls Jenkins** → Via curl HTTP request
4. **Jenkins fetches code** → From Git server to workspace
5. **Build stage** → Compile Debug and Release configurations
6. **Static analysis** → Run cpp-check on application code
7. **Flash Debug** → Program Nucleo board with Debug image
8. **Test Debug** → Run HIL tests, verify build ID
9. **Flash Release** → Program Nucleo board with Release image
10. **Test Release** → Run HIL tests on Release build
11. **Deliver** → Copy validated images to release store (success only)
12. **Notify** → Send email if any stage fails

---

## Course Content by Lesson

### Lesson 1: Introduction to the Course
**Duration:** Introductory
**Type:** Conceptual

**Topics Covered:**
- Benefits of CI/CD for embedded systems (automation, early feedback, culture)
- System architecture overview
- Hardware and software requirements
- Course philosophy: "Start small, add incrementally"
- Continuous delivery vs continuous deployment

**Deliverables:**
- Understanding of CI/CD value proposition
- System architecture diagram comprehension
- Hardware/software shopping list

---

### Lesson 2: Creating STM32CubeIDE Project and Git Repo
**Duration:** Hands-on
**Type:** Setup & Configuration

**Topics Covered:**
- STM32CubeIDE project structure deep dive
  - IDE system components
  - IDE-provided libraries (HAL, LL, CMSIS)
  - Tool chains and build outputs
- What goes in Git repo vs what doesn't
- Creating `.gitignore` for embedded projects
- Setting up local Git repository
- Creating bare Git repository (remote)
- Git command-line basics (status, diff, log)
- Integrating application code with IDE-generated code
- Blue Pill board configuration

**Key Commands:**
```bash
git init
git config user.name/user.email
git add .
git commit -m "message"
git init --bare
git remote add origin <url>
git push -u origin master
```

**Deliverables:**
- Properly structured IDE project
- Local Git repository with application code
- Remote bare repository
- `.gitignore` file configured for embedded builds
- Both Debug and Release configurations built

---

### Lesson 3: Automation of STM32CubeIDE Tasks
**Duration:** Hands-on
**Type:** Scripting & Automation

**Topics Covered:**
- Identifying tasks to automate (build, flash) vs manual tasks (design)
- Developer workflow for makefile management
- Build automation: Two options
  - Option 1: Headless IDE (not used)
  - Option 2: Make-based builds (selected)
- Extracting build environment from IDE
- Creating build scripts (Windows batch files)
- Build ID injection into firmware
- Flash programming with STM32_Programmer_CLI
- ST-Link serial number management
- Makefile path issues and solutions

**Key Scripts Created:**
- `build.bat` - Automated build script for Jenkins
- `build_i.bat` - Simplified build script for IDE use
- `flash.bat` - Automated flash programming
- `flash_i.bat` - Simplified flash script for IDE use

**Makefile Issue:**
- Problem: Full paths in makefiles break Jenkins builds
- Solution: Use relative paths for linker scripts

**Deliverables:**
- Working command-line build automation
- Working command-line flash programming
- Build ID injection mechanism
- Scripts ready for Jenkins integration

---

### Lesson 4: Static Code Analysis and HIL Testing
**Duration:** Hands-on
**Type:** Testing & Quality Assurance

**Topics Covered:**
- Static code analysis concepts
- cpp-check integration and configuration
- Suppressing false positives
- Deciding what code to analyze (app only vs all code)
- Types of automated testing (unit, integration, HIL)
- HIL testing philosophy and value
- Build ID verification importance (Ring doorbell story)
- HIL test hardware setup and connections
- GPIO command processor application
- Python test script architecture
- Data-driven test design
- JUnit XML report generation
- p-expect for stimulus-response testing

**Application Commands:**
```
config <port> <pin> <direction>  # 0=input, 1=output
read <port> <pin>
write <port> <pin> <value>
reset
version
```

**Test Strategy:**
1. Verify console prompt (software booted)
2. Verify build version (testing correct software)
3. Run GPIO tests (hardware functionality)

**Critical Lesson: The Ring Doorbell Story**
- First product shipped before Christmas
- Video quality complaints immediately
- Investigation revealed: **Software changes were never tested**
- Testing setup problem: Wrong version was tested
- Lucky fix: Could update on cloud side
- **Lesson:** Always verify build ID in automated tests

**Deliverables:**
- Static analysis integration
- HIL test hardware setup
- GPIO test application
- Python test script (`base_hilt.py`)
- JUnit XML test reports

---

### Lesson 5: Setting up Jenkins
**Duration:** Hands-on
**Type:** CI/CD Integration

**Topics Covered:**
- Jenkins architecture and concepts
- Plugin-based architecture
- Freestyle vs Pipeline projects
- Scripted vs Declarative pipelines
- Jenkinsfile: Storage in Git vs Jenkins server
- Lean Jenkinsfile philosophy
- Jenkins installation on Windows
- Java JDK requirements and `JAVA_HOME`
- Windows security configuration for Jenkins service
- Email notification configuration (SMTP)
- Parameterized builds for hardware configuration
- Generic Web Hook Trigger configuration
- Pipeline configuration from SCM
- Declarative Jenkinsfile structure
- Git post-update hooks
- Reading Jenkins dashboard and build results

**Jenkins Configuration:**
- **HTTP Interface:** `http://localhost:8080`
- **Service:** Runs as Windows background service
- **Plugins:** Generic Web Hook, Extended Email, JUnit

**Jenkinsfile Structure:**
```groovy
pipeline {
    environment { }
    stages {
        stage('Build') { }
        stage('Static Analysis') { }
        stage('Flash Debug') { }
        stage('Test Debug') { }
        stage('Flash Release') { }
        stage('Test Release') { }
    }
    post {
        success { }
        unsuccessful { }
    }
}
```

**Build Parameters:**
- `COM_DUT` - COM port for Device Under Test
- `COM_SIM` - COM port for HIL simulation hardware
- `STLINK_SN` - ST-Link serial number

**Web Hook Token:** `cicd_class_1`

**Deliverables:**
- Fully configured Jenkins installation
- Working pipeline triggered by Git pushes
- Email notifications on build failures
- Parameterized builds for hardware flexibility
- Test results displayed in Jenkins UI

---

### Lesson 6: Next Steps
**Duration:** Discussion
**Type:** Advanced Topics & Production Considerations

**Topics Covered:**
- Moving from demo to production system
- Replacing file-based Git with network Git server or GitHub
- Preventing bad code from entering official branch
- Pipeline reliability requirements for team acceptance
- Security improvements (key-based auth, certificates)
- Multi-developer considerations
- IDE settings file challenges
  - Local environment data in settings files
  - Non-identical but logically equivalent files
  - "Whipsawed" settings files problem
- Layered repository architecture
  - Layer 1: Application code (hand-written)
  - Layer 2: IDE-maintained files (CI/CD build files)
  - Layer 3: Per-developer settings

**Production Improvements:**
1. Network-based Git server (SSH URLs)
2. GitHub integration with CI/CD
3. Key-based authentication
4. Certificate-based email
5. Gating official branch on pipeline success

**Multi-Developer Solutions:**
- **Simple:** Standardize project locations across all developers
- **Advanced:** Layered repository architecture for separation of concerns

**Deliverables:**
- Understanding of production-grade requirements
- Strategies for multi-developer teams
- Architectural patterns for IDE-based projects

---

## Key Technical Concepts

### Build ID Injection
**Purpose:** Embed unique build identifier in firmware to verify correct version during testing

**Implementation:**
```batch
echo #define BUILD_VERSION "%BUILD_ID%" > version.h
```

**Usage in Application:**
```c
#include "version.h"
// Console command prints BUILD_VERSION
```

### Makefile Management Workflow
**Challenge:** IDE generates makefiles, but updates only happen during IDE builds

**Solution:** Developer workflow ensures makefiles stay current
1. Make code changes
2. Build and test in IDE (iterate)
3. **Before committing:** Build BOTH Debug AND Release in IDE
4. Commit both code changes AND updated makefiles
5. Push to Git

### Static Analysis Integration
**Tool:** cpp-check
**Scope:** Application code only (not third-party or IDE-supplied code)
**Behavior:** Failures mark build "unstable" but don't stop pipeline
**False Positives:** Can suppress with `// cppcheck-suppress <rule>` comments

### HIL Test Architecture
**DUT (Device Under Test):** Nucleo board running firmware
**SIM (Simulation Hardware):** Blue Pill simulating external hardware
**Test Script:** Python with p-expect, communicates via serial to both boards
**Validation:** Tests verify GPIO signals pass between boards correctly

### Jenkins Pipeline Error Handling
- **Build failures:** Stop pipeline immediately
- **Static analysis failures:** Mark "unstable", continue pipeline
- **Test failures:** Stop pipeline, send email
- **Success:** Execute delivery, copy images to release store

---

## Files and Directory Structure

### IDE Project Structure
```
cicd_class_1/
├── .settings/               # IDE settings
├── app/                     # Application code (hand-written)
│   └── gpioapp/
│       ├── appmain.c
│       └── config.h
├── cicd_tools/              # Automation scripts
│   ├── build.bat
│   ├── build_i.bat
│   ├── flash.bat
│   ├── flash_i.bat
│   ├── base_hilt.py
│   ├── static_analysis.bat
│   ├── check_test_results.bat
│   └── deliver.bat
├── Core/                    # IDE-generated code
│   ├── Inc/
│   └── Src/
│       └── main.c           # Modified to call app_main()
├── Drivers/                 # IDE-supplied HAL/LL libraries
├── Debug/                   # Debug build directory
│   ├── makefiles (Git)
│   └── *.o, *.elf (not in Git)
├── Release/                 # Release build directory
│   ├── makefiles (Git)
│   └── *.o, *.elf (not in Git)
├── .gitignore               # Build output exclusions
├── .cproject                # IDE project file
├── .project                 # IDE project file
├── .mxproject               # CubeMX project file
├── cicd_class_1.ioc         # Hardware configuration
├── STM32F401RETX_FLASH.ld   # Linker script
└── Jenkinsfile              # Pipeline definition
```

### Git Repository Structure
```
C:/Users/gene/repos/
└── cicd_class_1.git/        # Bare repository (remote)
    ├── hooks/
    │   └── post-update      # Web hook script
    ├── objects/
    ├── refs/
    └── ...
```

### Jenkins Workspace
```
C:/Users/gene/.jenkins/workspace/cicd_class_1/
├── (Git repo contents fetched here)
└── test_results*.xml        # Generated during pipeline
```

---

## Prerequisites & Background Knowledge

### Required Knowledge
- Basic understanding of Git concepts (repositories, commits, push/pull)
- Familiarity with Windows command line
- Basic understanding of embedded systems concepts
- Awareness of ARM Cortex-M architecture (helpful but not critical)

### Optional Knowledge (Helpful)
- Windows batch scripting
- Python programming
- STM32CubeIDE usage
- Jenkins concepts
- Make build system

### Not Required
- Deep C programming expertise
- Advanced Git operations
- Jenkins administration
- Python expert-level knowledge

---

## Learning Approach

### Course Philosophy
1. **Start Small:** "Doing just a little is better than doing nothing"
2. **Incremental Build-Up:** Add features over time as needed
3. **Practical Focus:** Everything demonstrated with real hardware
4. **Tool Independence:** Lean Jenkinsfile, logic in separate scripts
5. **Development Culture:** Build automation mindset into daily workflow

### Hands-On Methodology
- Live demonstrations with real hardware
- Command-line emphasis (transparency and confidence)
- Troubleshooting real issues encountered
- Instructor shares past experiences and lessons learned
- GitHub repositories provided for reference

### Best Practices Emphasized
1. Always verify build ID during testing
2. Keep makefiles current (build both Debug and Release before commit)
3. Use command-line tools for transparency
4. Store Jenkinsfile with source code in Git
5. Keep Jenkinsfile lean, delegate work to scripts
6. Even minimal HIL testing prevents major problems
7. Pipeline reliability is critical for team adoption

---

## Success Metrics

### By End of Course, Students Can:
1. ✅ Set up Git repositories for embedded projects with IDE-generated files
2. ✅ Automate embedded builds from command line
3. ✅ Automate flash programming from scripts
4. ✅ Integrate static code analysis into workflow
5. ✅ Design and implement HIL test systems
6. ✅ Configure Jenkins for embedded CI/CD
7. ✅ Create declarative pipelines with Jenkinsfile
8. ✅ Set up Git web hooks for automatic builds
9. ✅ Generate and interpret test reports
10. ✅ Deliver validated firmware releases automatically

### Deliverable CI/CD System Features
- ✅ Automatic builds on Git push
- ✅ Static code analysis with cpp-check
- ✅ Automated flash programming
- ✅ HIL testing with actual hardware
- ✅ Build ID verification
- ✅ JUnit test reporting
- ✅ Email notifications on failures
- ✅ Automated delivery of validated releases
- ✅ Dashboard visibility of build status

---

## Resources & References

### GitHub Repositories (Mentioned by Instructor)
- Main Nucleo project repository
- Blue Pill HIL simulation hardware project

### Documentation to Reference
- STM32CubeIDE documentation
- Jenkins documentation (declarative pipelines)
- Git documentation
- Python p-expect module documentation
- cpp-check documentation
- STM32_Programmer_CLI user manual

### Tools to Download
- STM32CubeIDE: [st.com](https://www.st.com/en/development-tools/stm32cubeide.html)
- STM32CubeProgrammer: [st.com](https://www.st.com/en/development-tools/stm32cubeprog.html)
- Jenkins: [jenkins.io](https://www.jenkins.io/)
- Git: [git-scm.com](https://git-scm.com/)
- Python 3: [python.org](https://www.python.org/)
- Java JDK: Specific compatible version for Jenkins

---

## Production Deployment Considerations

### Security Hardening
- Replace password authentication with SSH keys
- Use certificate-based email authentication
- Implement proper access control on Jenkins
- Secure Git server with proper authentication

### Scalability for Teams
- Network-based Git server (not file-based)
- Multiple build agents for parallel builds
- Dedicated HIL testing server with REST API
- Cloud-based CI/CD (GitHub Actions, GitLab CI, etc.)

### Advanced Features (Beyond Course Scope)
- Automated IDE code regeneration
- Automated makefile updates
- Branch protection rules
- Code review integration
- Multiple hardware configurations
- Automated regression testing
- Performance benchmarking
- Code coverage analysis
- Integration with issue trackers

---

## Instructor's Key Insights

### From Real-World Experience
1. **Ring Doorbell Lesson:** Always verify build ID - shipped untested software nearly killed product
2. **Pipeline Reliability:** Unreliable pipelines frustrate developers and hurt productivity
3. **Start Small Philosophy:** Even just static analysis adds value; build incrementally
4. **Tool Independence:** Make-based builds more reliable than headless IDE
5. **HIL Testing Value:** Even minimal testing (just boot check) prevents embarrassing failures
6. **Makefile Management:** Developer workflow can handle manual makefile updates acceptably
7. **Multi-Developer Challenges:** IDE settings files can cause subtle issues in teams

### Course Design Decisions
- **GUI IDE:** Demonstrates CI/CD possible even with GUI tools (not just command-line IDEs)
- **Single Laptop:** Everything on one machine for simplicity (production would use servers)
- **Make-based Builds:** Chose make over headless IDE for greater reliability
- **Command-Line Git:** Transparency and confidence over IDE plugin convenience
- **Lean Jenkinsfile:** Keep pipeline simple, delegate work to separate scripts
- **File-based Git:** Simplicity for demo (production needs network-based server)

---

## Summary

This course provides a complete, practical introduction to CI/CD for embedded systems, demonstrating that automation is achievable even with GUI-based development tools. By building a working system with real hardware, students gain hands-on experience with all aspects of embedded CI/CD: source control, build automation, static analysis, flash programming, HIL testing, and pipeline orchestration with Jenkins.

The course emphasizes practical solutions, incremental improvement, and building an automation mindset into daily development workflows. Real-world examples (particularly the Ring doorbell story) underscore the critical importance of automated testing and build ID verification.

**Key Takeaway:** Start small, automate what you can, and build up your CI/CD capabilities over time. Even minimal automation provides significant value in embedded development.

---

**Course Materials:** Available on GitHub (repositories mentioned by instructor)
**Target Audience:** Embedded software engineers, firmware developers, DevOps engineers working with embedded systems
**Skill Level:** Intermediate (basic embedded and Git knowledge assumed)
**Completion Time:** Self-paced, approximately 8-12 hours for full implementation
