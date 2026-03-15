# GdBot

GdBot is a Geode-based Geometry Dash mod scaffold intended for an in-process microbot and overlay workflow.

The current build is intentionally conservative:

- It proves the mod loads through a lightweight `MenuLayer` hook.
- It exposes a placeholder OpenGL-oriented UI entry point from inside Geometry Dash.
- It keeps the bot and overlay logic separated so input automation and rendering code do not bleed into hooks.

The actual Dear ImGui renderer bridge is still intentionally not wired yet. The current scaffold explicitly points at an OpenGL-first overlay backend, because starting with a DX11-specific path would be the wrong abstraction to lock in early.
