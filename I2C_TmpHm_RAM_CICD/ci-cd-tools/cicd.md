# Batch File Basics

## Essential Syntax & Structure

Based on `buildi.bat` and `flashi.bat` examples.

---

## 1. File Structure

```batch
@echo off                    REM Hide commands from output
set "usage=usage: script"    REM Define usage message
setlocal                     REM Keep variables local to this script

REM Your script logic here

call other_script.bat        REM Call another batch file
exit /b 0                    REM Exit with code (optional)
```

---

## 2. Comments

```batch
rem This is a comment
REM This is also a comment (case insensitive)
```

---

## 3. Variables

### Setting Variables
```batch
set "var_name=value"              REM Good practice (handles spaces)
set var_name=value                REM Also works
```

### Using Variables
```batch
echo %var_name%                   REM Display variable value
set "new_var=%var_name%\subfolder" REM Use in another variable
```

### Command-line Arguments
```batch
%1    REM First argument
%2    REM Second argument
%3    REM Third argument
%~dp0 REM Directory where this script is located
```

### Example from buildi.bat:
```batch
set "build_type=%1"    REM Capture first argument
set "target=%2"        REM Capture second argument
```

---

## 4. Conditional Statements

### Check if argument exists
```batch
if not [%1]==[] goto :label_name
```
From `buildi.bat`:
```batch
if not [%1]==[] goto :check_for_args
set "build_type=Debug"    REM Default if no argument
```

### Check if variable is defined
```batch
if defined WORKSPACE (
    set "ws_root=%WORKSPACE%\I2C_TmpHm_RAM_CICD"
) else (
    set "ws_root=C:\Users\Sheen\..."
)
```

### Check if file exists
```batch
if exist "file.txt" (
    echo File exists
) else (
    echo File not found
)
```

---

## 5. Labels & Goto

Labels are like bookmarks in your script.

```batch
goto :label_name    REM Jump to label

:label_name        REM Define label
echo "At label"
```

From `buildi.bat`:
```batch
if not [%1]==[] goto :check_for_args
set "build_type=Debug"
goto :set_build_dir

:check_for_args
set "build_type=%1"

:set_build_dir
set "build_dir=%ws_root%\%build_type%"
```

---

## 6. Calling Other Batch Files

### Without `call` (control doesn't return)
```batch
other_script.bat
REM This line won't execute if above script exits
```

### With `call` (control returns)
```batch
call other_script.bat
REM This line WILL execute after other_script finishes
```

From `buildi.bat`:
```batch
call "%script_dir%build.bat" "%build_dir%" %build_type% %target%
```

---

## 7. Special Commands

### `setlocal`
Keeps variables local to the script (doesn't affect parent environment)
```batch
setlocal
set "my_var=value"    REM Only exists in this script
```

### `@echo off`
Hides command execution from output
```batch
@echo off    REM Don't show commands
echo Hello   REM Only shows "Hello", not "echo Hello"
```

### `rem @echo off`
Comment it out to see all commands (useful for debugging)
```batch
rem @echo off    REM Commands will be visible
```

---

## 8. Quoting Paths

Always quote paths with spaces:
```batch
set "path=C:\Program Files\MyApp"           REM Good
call "%path%\script.bat"                    REM Good

set path=C:\Program Files\MyApp             REM Bad (breaks on space)
```

---

## 9. Common Patterns from Your Files

### Pattern 1: Default Arguments (flashi.bat)
```batch
set "build_type=Debug"                REM Set default
if not [%1]==[] set "build_type=%1"   REM Override if argument provided
```

### Pattern 2: Workspace Detection (buildi.bat)
```batch
if defined WORKSPACE (
    set "ws_root=%WORKSPACE%\I2C_TmpHm_RAM_CICD"
) else (
    set "ws_root=C:\Users\Sheen\Desktop\..."
)
```

### Pattern 3: Script Directory
```batch
set "script_dir=%~dp0"                      REM Get this script's directory
call "%script_dir%build.bat" parameters     REM Call script in same folder
```

---

## 10. Quick Reference: Your Scripts Explained

### buildi.bat Structure
```batch
@echo off                              REM Hide commands
set "usage=..."                        REM Define usage
setlocal                               REM Local scope

if not [%1]==[] goto :check_for_args   REM If args exist, jump
set "build_type=Debug"                 REM Default: Debug
set "target=all"                       REM Default: all
goto :set_build_dir                    REM Skip to build dir setup

:check_for_args                        REM Arguments provided
set "build_type=%1"                    REM Use arg 1
set "target=%2"                        REM Use arg 2

:set_build_dir                         REM Set paths
if defined WORKSPACE (...)             REM Jenkins or local?
set "build_dir=..."                    REM Build directory
set "script_dir=%~dp0"                 REM This script's location

call "%script_dir%build.bat" ...       REM Call actual build script
```

### flashi.bat Structure
```batch
rem @echo off                          REM Commented = show commands
set "usage=..."                        REM Define usage
setlocal                               REM Local scope

set "build_type=Debug"                 REM Default value
if not [%1]==[] set "build_type=%1"    REM Override if provided

set "ws_root=..."                      REM Workspace path
set "sn=..."                           REM Serial number for device
set "image_file=..."                   REM Path to .bin file

"%ws_root%\ci-cd-tools\flash.bat" ...  REM Call flash script
```

---

## 11. Common Issues & Tips

### Issue: Script stops after calling another script
**Solution:** Use `call` before the script name
```batch
call script.bat    REM Good
script.bat         REM Bad (control doesn't return)
```

### Issue: Variables with spaces break
**Solution:** Always quote variables
```batch
set "path=C:\Program Files"    REM Good
call "%path%\app.bat"          REM Good
```

### Issue: Can't debug what's happening
**Solution:** Comment out `@echo off` temporarily
```batch
rem @echo off    REM Now you see all commands
```

---

## 12. Testing Your Scripts

Run with different arguments:
```batch
buildi.bat                 REM Uses defaults: Debug all
buildi.bat Release all     REM Custom: Release all
buildi.bat Debug clean     REM Custom: Debug clean

flashi.bat                 REM Uses default: Debug
flashi.bat Release         REM Custom: Release
```

---

## Summary Cheat Sheet

| Syntax | Purpose | Example |
|--------|---------|---------|
| `rem` | Comment | `rem This is a comment` |
| `set "var=value"` | Set variable | `set "name=John"` |
| `%var%` | Use variable | `echo %name%` |
| `%1`, `%2` | Arguments | `set "type=%1"` |
| `%~dp0` | Script directory | `set "dir=%~dp0"` |
| `if` | Conditional | `if exist file.txt (...)` |
| `goto :label` | Jump to label | `goto :start` |
| `:label` | Define label | `:start` |
| `call` | Call script | `call other.bat` |
| `setlocal` | Local scope | `setlocal` |
| `@echo off` | Hide commands | `@echo off` |
| `exit /b` | Exit script | `exit /b 0` |

---
---

# Jenkins Pipeline Basics

## Essential Syntax & Structure

Based on `Jenkinsfile_HelloWorld` example.

---

## 1. Jenkinsfile Structure

```groovy
pipeline {                    // Main pipeline block
    agent any                 // Where to run (any available agent)

    environment {             // Define environment variables
        VAR_NAME = "value"
    }

    stages {                  // Container for all stages
        stage('Stage Name') { // Individual stage
            steps {           // Steps to execute
                echo 'Message'
                bat 'command'
            }
        }
    }
}
```

---

## 2. Key Components

### `pipeline { }`
The top-level block that contains the entire pipeline definition.
```groovy
pipeline {
    // Everything goes here
}
```

### `agent any`
Specifies where the pipeline runs. `any` means Jenkins picks any available agent.
```groovy
agent any              // Run on any available agent
agent { label 'windows' }  // Run on specific agent
```

### `environment { }`
Define environment variables accessible throughout the pipeline.
```groovy
environment {
    TOOL_DIR = "${WORKSPACE}\\I2C_TmpHm_RAM_CICD\\ci-cd-tools"
    BUILD_TYPE = "Debug"
}
```

### `stages { }`
Container that holds all your stage blocks.
```groovy
stages {
    stage('Build') { ... }
    stage('Test') { ... }
}
```

### `stage('Name') { }`
Represents a distinct phase in your pipeline (Build, Test, Deploy, etc.).
```groovy
stage('Build') {
    steps {
        // Build commands
    }
}
```

### `steps { }`
Contains the actual commands to execute in a stage.
```groovy
steps {
    echo 'Building...'
    bat 'build.bat'
}
```

---

## 3. Built-in Variables

Jenkins provides special variables:

| Variable | Description | Example |
|----------|-------------|---------|
| `${WORKSPACE}` | Current workspace directory | `C:\ProgramData\Jenkins\.jenkins\workspace\MyJob` |
| `${BUILD_NUMBER}` | Current build number | `42` |
| `${JOB_NAME}` | Name of the job | `I2C_TmpHm_RAM_CICD` |
| `${BUILD_ID}` | Unique build identifier | `2023-01-15_12-34-56` |

From `Jenkinsfile_HelloWorld`:
```groovy
environment {
    TOOL_DIR = "${WORKSPACE}\\I2C_TmpHm_RAM_CICD\\ci-cd-tools"
}
```

---

## 4. Common Commands in Steps

### `echo`
Print messages to console output.
```groovy
echo 'Hello World'
echo "Build type: ${BUILD_TYPE}"
```

### `bat` (Windows)
Execute Windows batch commands or scripts.
```groovy
bat 'buildi.bat Debug all'
bat "\"${TOOL_DIR}\\buildi.bat\" Debug all"
bat 'dir'
```

### `sh` (Linux/Mac)
Execute shell commands (not used in Windows).
```groovy
sh './build.sh'
sh 'make all'
```

### `junit`
Publish JUnit test results.
```groovy
junit 'test-results.xml'
junit '**/test-results/*.xml'
```

---

## 5. Comments

```groovy
// This is a single-line comment

/* This is a
   multi-line comment */
```

From `Jenkinsfile_HelloWorld`:
```groovy
// bat "\"${TOOL_DIR}\\check-test-results.bat\" badweh-test-results.xml"
```

---

## 6. String Quoting

### Single Quotes `'...'`
Literal strings (no variable interpolation).
```groovy
echo 'Hello World'           // Prints: Hello World
echo 'Value: ${VAR}'         // Prints: Value: ${VAR}
```

### Double Quotes `"..."`
Allows variable interpolation with `${...}`.
```groovy
echo "Value: ${BUILD_TYPE}"  // Prints: Value: Debug
bat "\"${TOOL_DIR}\\build.bat\""
```

### Escaping Quotes in Commands
When calling batch files with paths containing spaces, escape quotes:
```groovy
bat "\"${TOOL_DIR}\\buildi.bat\" Debug all"
// Becomes: "C:\path\to\buildi.bat" Debug all
```

---

## 7. Your Jenkinsfile Explained

### Complete Structure
```groovy
pipeline {
    agent any    // Run on any available Jenkins agent

    environment {
        // Set TOOL_DIR to workspace's ci-cd-tools folder
        TOOL_DIR = "${WORKSPACE}\\I2C_TmpHm_RAM_CICD\\ci-cd-tools"
    }

    stages {
        // Stage 1: Hello message
        stage('Hello C') {
            steps {
                echo 'Hello World C.'
            }
        }

        // Stage 2: Build the firmware
        stage('Build') {
            steps {
                echo 'Explore Build Stage.'
                bat "\"${TOOL_DIR}\\buildi.bat\" Debug all"
            }
        }

        // Stage 3: Flash firmware to device
        stage('Flash') {
            steps {
                echo 'Explore Flash Stage.'
                bat "\"${TOOL_DIR}\\flashi.bat\""
            }
        }

        // Stage 4: Run tests and publish results
        stage('Test') {
            steps {
                bat "python \"${TOOL_DIR}\\MBC_HIL_Full_Suit_Advanced.py\" "
                junit 'test-results.xml'
                // Commented out: check-test-results.bat
            }
        }
    }
}
```

### Execution Flow
1. **Hello C** stage: Prints greeting
2. **Build** stage: Runs `buildi.bat Debug all`
3. **Flash** stage: Runs `flashi.bat`
4. **Test** stage: Runs Python test script and publishes JUnit results

---

## 8. Common Pipeline Patterns

### Pattern 1: Sequential Stages
Stages run one after another (default behavior).
```groovy
stages {
    stage('Build') { ... }    // Runs first
    stage('Test') { ... }     // Runs second
    stage('Deploy') { ... }   // Runs third
}
```

### Pattern 2: Environment Variables
Define once, use everywhere.
```groovy
environment {
    BUILD_DIR = "${WORKSPACE}\\Debug"
    CONFIG = "Debug"
}

stages {
    stage('Build') {
        steps {
            bat "build.bat ${CONFIG}"  // Uses Debug
        }
    }
}
```

### Pattern 3: Multiple Commands in Steps
```groovy
steps {
    echo 'Starting build...'
    bat 'buildi.bat Debug all'
    echo 'Build complete!'
}
```

### Pattern 4: Test Result Publishing
```groovy
steps {
    bat 'run-tests.bat'        // Generate test-results.xml
    junit 'test-results.xml'   // Publish results to Jenkins
}
```

---

## 9. File Paths in Jenkins

### Windows Paths
Use double backslashes `\\` in strings:
```groovy
"${WORKSPACE}\\I2C_TmpHm_RAM_CICD\\ci-cd-tools"
```

### Why Escape Quotes?
Batch scripts with spaces in paths need quoted paths:
```groovy
bat "\"${TOOL_DIR}\\buildi.bat\" Debug all"
```
This becomes:
```batch
"C:\path with spaces\buildi.bat" Debug all
```

---

## 10. Running Your Jenkinsfile

### Option 1: Jenkins Server
1. Create a Pipeline job in Jenkins
2. Point to your Jenkinsfile (SCM or paste directly)
3. Run the job

### Option 2: Replay (for testing)
1. Go to a previous build
2. Click "Replay"
3. Modify Jenkinsfile and run

### Option 3: Test Locally with Batch Scripts
Since your Jenkinsfile just calls batch scripts, test them directly:
```batch
cd C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD

REM Simulate Jenkins stages
.\I2C_TmpHm_RAM_CICD\ci-cd-tools\buildi.bat Debug all
.\I2C_TmpHm_RAM_CICD\ci-cd-tools\flashi.bat
python .\I2C_TmpHm_RAM_CICD\ci-cd-tools\MBC_HIL_Full_Suit_Advanced.py
```

---

## 11. Common Issues & Tips

### Issue: Path not found
**Problem:** `${WORKSPACE}` doesn't exist locally
```groovy
bat "\"${TOOL_DIR}\\buildi.bat\""  // TOOL_DIR uses ${WORKSPACE}
```
**Solution:** Your batch files handle this with WORKSPACE detection:
```batch
if defined WORKSPACE (
    set "ws_root=%WORKSPACE%\I2C_TmpHm_RAM_CICD"
) else (
    set "ws_root=C:\Users\Sheen\Desktop\..."
)
```

### Issue: Stage fails but pipeline continues
**Solution:** Jenkins stops at first failure by default (good!)

### Issue: Can't see batch script output
**Solution:** Ensure `@echo off` is commented in .bat files during debugging:
```batch
rem @echo off    REM See all commands
```

### Issue: Test results not showing
**Problem:** Wrong file path in `junit`
```groovy
junit 'test-results.xml'  // Must be in workspace root
```
**Solution:** Verify your test script generates `test-results.xml` in the right location.

---

## 12. Jenkins Pipeline Cheat Sheet

| Syntax | Purpose | Example |
|--------|---------|---------|
| `pipeline { }` | Main container | `pipeline { agent any }` |
| `agent any` | Where to run | `agent { label 'windows' }` |
| `environment { }` | Define variables | `VAR = "value"` |
| `${VAR}` | Use variable | `"Path: ${WORKSPACE}"` |
| `stages { }` | Container for stages | `stages { stage(...) }` |
| `stage('Name') { }` | Define stage | `stage('Build') { }` |
| `steps { }` | Commands to run | `steps { bat 'build.bat' }` |
| `echo` | Print message | `echo 'Hello'` |
| `bat` | Windows command | `bat 'dir'` |
| `sh` | Linux/Mac command | `sh './build.sh'` |
| `junit` | Publish tests | `junit '*.xml'` |
| `//` | Comment | `// This is a comment` |
| `'...'` | Literal string | `'Hello ${VAR}'` (no interpolation) |
| `"..."` | Interpolated string | `"Hello ${VAR}"` (with interpolation) |

---

## 13. Comparison: Batch vs Jenkins

| Aspect | Batch File | Jenkinsfile |
|--------|------------|-------------|
| Language | Batch script | Groovy (DSL) |
| Variables | `%VAR%` | `${VAR}` |
| Comments | `rem` | `//` or `/* */` |
| Run command | `call script.bat` | `bat 'script.bat'` |
| If statement | `if exist file (...)` | `if (fileExists('file'))` |
| Environment | `set "VAR=value"` | `environment { VAR = "value" }` |
| Echo | `echo message` | `echo 'message'` |

---

## 14. Next Steps

1. **Test locally**: Run batch scripts directly before using Jenkins
2. **Start simple**: Begin with just the Build stage
3. **Add stages incrementally**: Add Flash, then Test
4. **Monitor Jenkins console**: Watch output to debug issues
5. **Use echo statements**: Debug with `echo "VAR = ${VAR}"`
