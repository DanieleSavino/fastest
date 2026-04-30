# -- ANSI colour helpers --------------------------------------------------------
_ANSI = {
    "reset":   "\033[0m",
    "green":   "\033[32m",
    "red":     "\033[31m",
    "yellow":  "\033[33m",
    "blue":    "\033[34m",
    "magenta": "\033[35m",
    "cyan":    "\033[36m",
    "dim":     "\033[2m",
    "bold":    "\033[1m",
}

def color(text: str, colour: str) -> str:
    """Wrap text in ANSI colour codes (no-op if colour unknown)."""
    code = _ANSI.get(colour, "")
    return f"{code}{text}{_ANSI['reset']}" if code else text
