## Introduction

This repository contains the artifact for the paper *Characterizing and Detecting Bugs at the Interfaces of OpenBSD Privilege-Separated Programs*.

The artifact has three parts:

- bug reports for all the compartment interface vulnerabilities (CIVs) we found;
- a fuzzing framework and harnesses for finding CIVs; and
- tools and prompts for LLM-assisted analysis.

## Requirements

**Hardware:** Fuzzing requires an x86-64 computer that runs Linux and has at least 16 GB of RAM and four CPU cores. However, we recommend use a computer with more cores if fuzzing campaigns will be run in parallel.

**Software:** The fuzzing environment runs on FreeBSD. We provide a FreeBSD virtual machine image that runs under QEMU. The host computer therefore needs QEMU and libvirt (`virsh`). The LLM evaluation uses OpenClaw, and we provide a script that installs it. We recommend Ubuntu 24.04 LTS, the system we used, to avoid compatibility problems across Linux distributions.

**LLM-access requirements**: An LLM API key is required to evaluate the LLM CIV analysis. We also recommend that the user go through the provider's identity verification program, to reduce the chance that responses get flagged by the LLM safety monitor.

## Major claims

This artifact supports the following three claims:

**C1:** OpenBSD programs still contain many CIVs, and these CIVs follow generalizable patterns (Table 3).

**C2:** An off-the-shelf fuzzer (AFL++) can efficiently detect CIVs when combined with our fuzzing harnesses (Section 5.1).

**C3:** LLMs can efficiently detect CIVs (Section 5.2).

## Evaluation

This section explains how to reproduce the experiments that support these claims.

### (C1) Examining CIVs in OpenBSD

This step does not require running any code. The `bug-reports/` directory contains all the bug reports we sent to the OpenBSD developers. These reports correspond to the CIVs marked F, L, and M in Table 3. Each report describes the bug in enough detail to derive the properties listed in Table 3, such as its root cause. We removed the proofs of concept for ethical reasons.

[`bug-reports/bugid.md`](bug-reports/bugid.md) maps the bug IDs in Table 3 to their reports.

### (C2) Fuzzing

We provide the source code for all the programs we fuzzed, along with our fuzzing framework and tools.

#### Fuzzing framework

Our framework implements the common parts of the fuzzing harness shown in Table 1. Its source code is in `fuzzing/ptr_checker/`. Documentation is in `fuzzing/ptr_checker/msg_generator_docs/`, and a compiled HTML version is available at `fuzzing/ptr_checker/msg_generator_docs/index.html`. We do not evaluate the framework separately. Instead, the per-program harnesses described below use it extensively and demonstrate its capabilities.

#### Per-program fuzzing harnesses

For each program, we provide a fuzzing harness and instructions for using the available detectors on FreeBSD. For programs that we ported, we also include the ported source code and its harness. These files are in `fuzzing/per-program-harness/`.

#### Fuzzing VM

We provide a FreeBSD VM that contains all the harnesses and a script that runs them automatically. The VM is in `fuzzing/vm/`. To use it:

1. Go to `fuzzing/vm/` and run `./connect.sh`. This script boots the VM and connects to it over SSH. Booting may take some time. Log in with the username `root` and the password `artifact`.
2. Run `/root/civ/fuzz.sh` in the VM. The script lists the 24 programs available for fuzzing and lets you choose a program and a detector configuration. Most programs provide two configurations:
   - **ASan + UBSan + pointer detector:** Detects MemSaf CIVs except leaks of uninitialized memory. It also detects pointer leaks (InfoLeak).
   - **MSan:** Detects leaks of uninitialized memory (MemSaf).
3. The AFL++ interface appears when the campaign starts. It reports detected CIVs as crashes. The campaign stores its results in the program's directory, under a directory whose name begins with `findings`.

We recommend starting with `tmux` and the ASan configuration. This setup can find a CIV within seconds.

### (C3) Searching for CIVs with an LLM

We provide prompts for detecting CIVs in `llm_prompts/info-leaks.md` and `llm_prompts/semantic-violation.md`. We also include a preliminary draft of the paper. We run these prompts with OpenClaw. We strongly recommend using an Ubuntu VM because the setup script performs a full OpenClaw installation, and it gives the agent full terminal control without approvals.

To set up the OpenClaw auditing environment:

1. Go to `llm_prompts/openclaw_setup/` and add your Claude API key to `openclaw.env`.
2. Run `./install-openclaw.sh`. The script installs OpenClaw and configures it to use Sonnet 4.6.
3. Export the OpenBSD 7.6 source tree into the agent workspace using CVS: `mkdir -p ~/openclaw-workspace && cd ~/openclaw-workspace && cvs -qd anoncvs@anoncvs.eu.openbsd.org:/cvs export -rOPENBSD_7_6_BASE -d src src`. Then copy the selected prompt and its required PDF inputs into `~/openclaw-workspace`.
4. Run `openclaw tui` to open the interactive chat interface.
5. Ask OpenClaw to follow one of the prompts. Each prompt performs a different analysis. Wait for OpenClaw to write its final report.
