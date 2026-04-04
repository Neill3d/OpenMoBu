# Contributing to OpenMoBu

Thank you for your interest in contributing to **OpenMoBu**!  
We welcome improvements in performance, stability, documentation, and new MotionBuilder-related features.  
This document explains how to propose changes, follow the code style, and participate in development.

---

## Table of Contents
1. [Reporting Issues](#1-reporting-issues)  
2. [Requesting Features](#2-requesting-features)  
3. [Forking & Branching Model](#3-forking--branching-model)  
4. [Code Style Guidelines](#4-code-style-guidelines)  
5. [Commit Message Conventions](#5-commit-message-conventions)  
6. [Pull Request Guidelines](#6-pull-request-guidelines)  
7. [Testing & Validation](#7-testing--validation)  
8. [Licensing](#9-licensing)  
9. [Code of Conduct](#10-code-of-conduct)

---

# 1. Reporting Issues

If you encounter a bug, please open a **GitHub Issue** and include:

- Clear title  
- Steps to reproduce  
- Expected vs actual behaviour  
- MotionBuilder version (e.g., 2022 / 2023 / 2024)  
- OS & GPU info (Windows, NVIDIA driver version, etc.)  
- Logs or screenshots (if relevant)

Before submitting, please search existing issues to avoid duplicates.

---

# 2. Requesting Features

Feature requests are welcome. When creating a feature request:

- Clearly describe the need or workflow improvement  
- Provide examples (videos, screenshots, use cases)  
- Add technical context where possible (API limits, MB SDK details)  
- Suggest a potential implementation direction (optional)

Large features should be discussed before implementation.

---

# 3. Forking & Branching Model

OpenMoBu uses a simple branching workflow:

- **master** → stable, production-ready  
- **feature/*** → for new features  
- **bugfix/*** → for bug fixes  
- **docs/*** → documentation updates

### Steps to contribute

1. Fork the repository  
2. Create a feature branch  
   ```bash
   git checkout -b feature/<short-description>
   ```  
3. Commit your changes  
4. Push to your fork  
5. Open a Pull Request against `master`

---

# 4. Code Style Guidelines

### General
- Use **modern C++17/20** features when appropriate  
- Keep functions small and readable  
- Avoid unnecessary dynamic allocations in real-time code

### File Naming
- `.cpp`, `.h`, `.inl`  
- PascalCase for classes and methods 
- camelCase for class members and variables  
- UPPER_CASE for constants

### Formatting
- 4-space indentation, no tabs  

### Error Handling
- Use assertions for internal logic assumptions  
- Avoid exceptions inside real-time render/evaluation loops  
- Prefer status return values for predictable control flow

---

# 5. Commit Message Conventions

Use descriptive commit messages. Recommended format:

```
<type>: <short summary>

[optional longer explanation]
```

**Types:**

- `feat:` new feature  
- `fix:` bug fix  
- `perf:` performance improvement  
- `refactor:` code cleanup  
- `docs:` documentation only  
- `build:` build/CI changes  
- `test:` tests

**Example:**

```
perf: optimize SSAO kernel sampling and reduce texture lookups
```

---

# 6. Pull Request Guidelines

Before opening a PR:

- Ensure the code builds on supported MotionBuilder versions  
- Verify the plugin loads and runs correctly  
- Run performance-sensitive features in a simple scene  
- Update documentation if the change affects the workflow (optional) 
- Add comments for complex GPU or multithreading logic

Your PR should include:

- Clear summary of changes  
- Screenshots or profiler results if relevant  
- Notes about compatibility (MB version, SDK differences)

Large PRs may be requested to be split.

---

# 7. Testing & Validation

OpenMoBu plugins interact closely with MotionBuilder.  
Before submitting:

### Functionality checks
- Rebuild the plugin in Release mode  
- Load in MotionBuilder  
- Test with a minimal scene  
- Verify stability across playback, scrubbing, and evaluation cycles

### Performance checks
- Use internal MotionBuilder's profiler tools or RenderDoc, Nsight, or any GPU profiler  
- Avoid regressions in real-time passes (post-processing, compute, etc.)

---

# 8. Licensing

By contributing, you agree that your code will be released under the project’s license.  
Ensure you have rights to contribute the code, especially if it originates from an employer.

---

# 9. Code of Conduct

This project follows the Contributor Covenant.  
Be respectful, constructive, and collaborative.

---
