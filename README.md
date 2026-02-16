# Action RPG Project

A demo-length third-person action RPG built in Unreal Engine 4 with C++, centered on
Souls-like combat and dungeon exploration. A centralized AI combat director coordinates
six enemy types — each with distinct behaviors, models, and animations — pacing their
engagements with the player. Character systems span a modular equipment architecture
across 15 slots, an inventory managing both slot capacity and carry weight, with a stamina
economy gating every combat action. Players start unarmed in dungeon depths and fight
upward, collecting gear, unlocking checkpoints, and building toward a boss encounter.

<p align="center">
  <a href="https://downloads.andrewcreekmore.dev/ActionRPGProject_1.0.zip">Download (Windows)</a>
  &nbsp;&nbsp;|&nbsp;&nbsp;
  <a href="https://andrewcreekmore.dev/projects/game/action-rpg-project">Portfolio</a>
</p>

<div align="center">
  <a href="https://github.com/andrewcreekmore/ActionRPGProject/assets/44483269/2562975b-3869-4975-81c3-2f6652d9df5b"><img src="https://github.com/andrewcreekmore/ActionRPGProject/assets/44483269/2562975b-3869-4975-81c3-2f6652d9df5b" width="24%" alt="Combat encounter" /></a>
  <a href="https://github.com/andrewcreekmore/ActionRPGProject/assets/44483269/463a7474-ae43-4dcc-98a2-de06a05e9e25"><img src="https://github.com/andrewcreekmore/ActionRPGProject/assets/44483269/463a7474-ae43-4dcc-98a2-de06a05e9e25" width="24%" alt="Enemy variety" /></a>
  <a href="https://github.com/andrewcreekmore/ActionRPGProject/assets/44483269/cb82dc0e-dbb4-42a1-b8fa-08550046edaf"><img src="https://github.com/andrewcreekmore/ActionRPGProject/assets/44483269/cb82dc0e-dbb4-42a1-b8fa-08550046edaf" width="24%" alt="Dungeon exploration" /></a>
  <a href="https://github.com/andrewcreekmore/ActionRPGProject/assets/44483269/1e4fde7c-db4a-414d-886f-f4094e5aa5b2"><img src="https://github.com/andrewcreekmore/ActionRPGProject/assets/44483269/1e4fde7c-db4a-414d-886f-f4094e5aa5b2" width="24%" alt="Boss encounter" /></a>
</div>

## Details

*Players fight through dungeon floors of increasingly dangerous enemies, equipping
collected weapons and armor, managing stamina for every combat action, and breaking
enemy posture to land finishing moves.*

- Six enemy AI types with distinct behaviors, coordinated by a centralized director that
  grades positioning and paces engagements
- Combat director scores each enemy by camera-relative positioning and time since last
  attack, issuing attack commands on a tunable delay to create paced, deliberate encounters
  rather than simultaneous rushes
- Poise and stagger system tracking cumulative hit impact — enemies stagger when poise
  breaks, opening a finishing-move window
- Stamina economy gating attacks, dodges, blocks, and sprints, with action-specific costs
  and recovery delays
- Modular equipment architecture across 15 character slots with data-driven stat effects
  and real-time mesh swapping
- Reusable inventory component tracking slot capacity and carry weight, shared across
  player and lootable containers

## Technology

- **Engine:** Unreal Engine 4
- **Languages:** C++
- **Visual Scripting:** Blueprint
- **Version Control:** Perforce

<p align="center"><a href="#action-rpg-project">↑ Back to top</a></p>
