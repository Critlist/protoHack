#!/usr/bin/env python3
"""
Shared color utilities for protoHack test tools
"""

class Colors:
    """ANSI color codes for output"""
    BLUE = '\033[1;36m'
    RED = '\033[1;31m'
    GREEN = '\033[1;32m'
    YELLOW = '\033[1;33m'
    RESET = '\033[0m'
    
    @classmethod
    def say(cls, tag: str, msg: str = "") -> None:
        print(f"{cls.BLUE}[{tag}]{cls.RESET} {msg}")
        
    @classmethod
    def error(cls, msg: str) -> None:
        print(f"{cls.RED}[ERROR]{cls.RESET} {msg}")
        
    @classmethod
    def success(cls, msg: str) -> None:
        print(f"{cls.GREEN}[SUCCESS]{cls.RESET} {msg}")
        
    @classmethod
    def warn(cls, msg: str) -> None:
        print(f"{cls.YELLOW}[WARN]{cls.RESET} {msg}")