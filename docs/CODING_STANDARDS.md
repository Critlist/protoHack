# Fenlason Hack Coding Standards

## Documentation Policy for Non-Original Code

This project preserves authentic 1982 Hack source code from Jay Fenlason and
collaborators (KW, MT, JP) at Lincoln-Sudbury Regional High School while making
minimal modern additions for buildability on modern Linux/gcc. To maintain clear
distinction between original and modern code:

### Documentation Requirement

**ALL** code that is not part of the original 1982 codebase or simple K&R to
ANSI C conversion MUST be documented with structured comments explaining:

1. **WHY** the addition was necessary
2. **HOW** it was implemented
3. **WHAT** it preserves from the original
4. **WHAT** it adds that's modern

### Documentation Format

For major additions (new functions, replaced subsystems):

```c
/**
 * MODERN ADDITION (2025): Brief description
 *
 * WHY: Explanation of the problem that required this addition
 *
 * HOW: Technical explanation of the implementation approach
 *
 * PRESERVES: What original 1982 behavior/logic is maintained
 * ADDS: What modern functionality or compatibility is provided
 */
```

For most changes (shorthand, preferred for typical work):

```c
/* Modern: Brief description */
```

### Examples of Code Requiring Documentation

- **New functions or logic** not present in original codebase
- **Bug fixes** that change behavior or add conditions (not just syntax fixes)
- **Compatibility shims** for modern systems (termios replacing sgtty, etc.)
- **Compatibility renames** to avoid library conflicts (e.g., `pow` → `hack_pow`)
- **Defensive checks** — bounds checks, null guards, resource cleanup not in original
- **Data structure or algorithm changes** (e.g., pointer arithmetic corrections)
- **Path changes** (HACKDIR replacing hardcoded `/usr/lib/games/hack`)
- **Added includes** for modern headers if they enable non-original behavior

### Examples of Code NOT Requiring Documentation

- **K&R to ANSI C conversion**: Function signature modernization
- **Simple syntax fixes**: Adding void, int return types for implicit-int
- **Header organization**: Moving declarations to proper includes
- **Build system**: CMake, compiler flags, compat.h
- **Whitespace/formatting**: Code style consistency

### Original Source Preservation Rule

### NEVER DELETE ORIGINAL 1982 CODE

All original source code must be preserved using one of these methods:

1. **Comment Preservation** (Preferred):

```c
#if 0
/* ORIGINAL 1982 CODE - preserved for reference */
original_function() {
    /* original implementation */
}
#endif
```

2. **Inline Comments** (For small changes):

```c
/* Original 1982: old_approach(); */
modern_approach();  /* Modern: explanation */
```

3. **Unmodified Originals**: The `original/` directory contains the complete
   unmodified original source tree and serves as the canonical reference.

**Rationale**: Enables future researchers to understand evolution, verify
authenticity claims, and potentially revert changes if needed.

### Preservation Principle

The goal is to make it easy for future developers to:

1. **Identify authentic 1982 code** vs modern additions
2. **Understand why** modern additions were necessary
3. **Evaluate whether** additions could be removed or improved
4. **Learn from** the historical code while understanding modern adaptations

### Preservation of Original Behavior

- **Preserve bugs** — original gameplay bugs are historical artifacts
- **Preserve misspellings** — "homonculous", "gelatenous", etc. are authentic
- **Preserve game balance** — do not alter item probabilities, monster stats, etc.
- **Preserve V7 idioms** — keep original logic even where a modern approach
  would be cleaner, unless it prevents compilation

### Project-Specific Notes

- **Two versions**: `src/root/` (export version) and `src/exp1/` (refactored
  version) are ported independently
- **Originals untouched**: `hack/` directory is never modified
- **compat.h**: Shared BSD-to-POSIX shim header lives in `src/`
- **mklev**: Remains a separate binary (original PDP-11 memory constraint
  design) — do not merge into the main hack binary

### Existing Documented Additions

As of January 2025, the following modern additions have been made:

1. **CMakeLists.txt**: Build system automating Jay's 11-step READ_ME setup
2. **src/compat.h**: BSD→POSIX shims (index→strchr, rindex→strrchr, standard includes)

All future additions should follow this documentation standard to maintain the
educational and preservation value of the project.
