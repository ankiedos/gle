# gle
General Language Environment version 2 implementation

# format:
- always LSB -> MSB
- memory module to use (imp, obj, decl, blkchn, reldb)
- instruction plugins to load
- performance config:
  - compactification frequency
  - compactification effectivity

# Roadmap
- add run-time disassembler in debug mode
- add emit() throwing error signals
- encode addressing modes in a single word