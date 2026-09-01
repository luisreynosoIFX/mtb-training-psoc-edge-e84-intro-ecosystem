# Presentation TODOs: Eclipse IDE to VS Code migration

Found by searching `PSE84_Introductory_Ecosystem_E2.pdf` (exported from the .pptx) for Eclipse/Quick Panel/Project Explorer references, and by inspecting slides whose titles imply an IDE screenshot. Speaker notes were not reviewed (no tool available to extract .pptx notes text in this session) — review them manually for the same terms.

- [ ] Slide 6 "Support for IDEs": IDE comparison table lists "Eclipse IDE for ModusToolbox" alongside VS Code, Command-line Interface, IAR Embedded Workbench, and Arm MDK with µVision. Update ordering/emphasis so VS Code is presented as the primary IDE, matching the manual.
- [ ] Slide 7 "Understanding ModusToolbox and Zephyr": bullet "IDE support, including Eclipse IDE, VS Code, and third-party IDEs" — confirm wording/order still reflects VS Code as the primary recommended IDE.
- [ ] Slide 8 "Overview of DEEPCRAFT Studio and ModusToolbox workflow": bullet "Project connector files in ModusToolbox applications with Quick Panel shortcuts for linking to DEEPCRAFT Studio Projects" — Quick Panel is an Eclipse-specific concept; update wording/screenshot to reference the VS Code ModusToolbox™ Assistant equivalent (Create Project / Application tab).
- [ ] Slide 12 "Tools utilized in this presentation": version list needs updating —
  - `ModusToolbox™ v3.6` → `ModusToolbox™ v3.9`
  - `Eclipse IDE for ModusToolbox™ v2025.4.0` → `Visual Studio Code + Infineon ModusToolbox™ for VS Code extension v1.10.0`
  - `Edge Protect Security Suite v1.6` → `Edge Protect Security Suite v2.2.0`
  - `ModusToolbox™ Programming Tools v1.5.0` → `ModusToolbox™ Programming Tools v1.9.0`
  - `KIT_PSE84_EVAL_EPC2 BSP v1.0.0` → `KIT_PSE84_EVAL_EPC2 BSP v1.4.0`
- [ ] Slide 13 "Project Creator": title-only slide, almost certainly an Eclipse Project Creator screenshot. Replace with a VS Code Project Creator / Create Project tab screenshot.
- [ ] Slide 15 "Library Manager": title-only slide, likely an Eclipse Library Manager screenshot. Replace with the VS Code (ModusToolbox™ Assistant → Application tab → Library Manager) screenshot, or the standalone Library Manager window if the tool UI itself is unchanged.
- [ ] Slide 35 "Programming/debugging": bullet says "Debug environment is IDE-specific"; confirm the accompanying example/screenshot doesn't show an Eclipse-specific debug launch configuration, and reference the VS Code Run and Debug view if needed.
- [ ] Slide 39 "Using Device Configurator": title-only slide, likely an Eclipse Device Configurator screenshot with Eclipse chrome (Quick Panel sidebar) visible. Replace with a VS Code screenshot, or verify the Device Configurator UI itself is IDE-agnostic and only the surrounding chrome needs updating.
- [ ] Review all slide notes for "Eclipse", "Quick Panel", "Project Explorer", and "Dashboard" and update narration to reference VS Code equivalents.
