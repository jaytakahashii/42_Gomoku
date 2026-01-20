<div align="center">

# 🧠 Gomoku - High-Performance AI Engine

![School](https://img.shields.io/badge/School-42_Paris-000000?style=flat-square&logo=42&logoColor=white&labelColor=24292e)
![Language](https://img.shields.io/badge/Language-C%2B%2B17-00599C?style=flat-square&logo=c%2B%2B&logoColor=white&labelColor=24292e)
![Algorithm](https://img.shields.io/badge/Algorithm-Minimax-FF4500?style=flat-square&labelColor=24292e)
![Optimization](https://img.shields.io/badge/Optimization-BitBoard-important?style=flat-square&labelColor=24292e)

![Score](https://img.shields.io/badge/Score-125%2F100-32a852?style=flat-square&labelColor=24292e)

<br />
<img src="./assets/Gomoku_demo.gif" alt="Gomoku AI Demo" width="600">
<br />

<p align="center">
  <strong>A highly optimized Gomoku AI capable of beating human players.</strong><br>
  Features 32-bit aligned BitBoards, Principal Variation Search (PVS), and Iterative Deepening.
</p>

[Report Bug](https://github.com/jaytakahashii/42_Gomoku/issues) · [Request Feature](https://github.com/jaytakahashii/42_Gomoku/issues)

</div>

---

## 📒 Introduction

This project is a C++ implementation of the strategy board game **Gomoku** (Five in a Row), developed as part of the curriculum at **42 Paris**.

The primary focus of this project is to build a highly optimized **AI engine** capable of beating human players within strict time constraints (avg. 0.5s/move). The game features a polished, retro-style GUI built with **SFML**, supporting custom rules such as _Capture_ and _Double-Three Forbidden_.

### 🤝 Collaboration

This project was a collaborative effort:

- **AI & Game Logic**: [Jay Takahashi](https://github.com/jaytakahashii) - Implemented the BitBoard engine, Minimax algorithm, and Heuristics.
- **UI & Graphics**: [Koji Watanabe](https://github.com/kojilbj) - Designed and implemented the SFML-based interface, Scene management, and visual effects.

---

## 🚀 Technical Highlights

The AI is built for speed and accuracy, leveraging low-level optimizations and advanced search algorithms.

### ⚡ 1. Optimization & Data Structure

- **32-bit Aligned BitBoards**: The 19x19 board is padded to a width of 32 bits. This alignment allows for **branchless boundary checks** using sentinel bits.
- **Bitwise Pattern Detection**: Line detection (5-in-a-row, open-4, etc.) and "Double-Three" validation are performed using SIMD-like bitwise operations (AND, SHIFT) instead of slow loop-based scanning.
- **Incremental Updates**: Uses **Zobrist Hashing** with XOR operations to update the board hash in $O(1)$ time during moves and undos.

### 🧠 2. Search Engine

- **PVS (Principal Variation Search)**: An enhancement of Alpha-Beta pruning. It searches the first move with a full window and assumes subsequent moves are inferior (Null Window Search), significantly reducing the search space.
- **Iterative Deepening**: Searches are performed at depth 1, 2, ... $N$. This is an investment to enable **Move Ordering**.
- **Transposition Table**: Caches board states and their evaluation scores. It serves two purposes:
  1.  **Exact Cutoff**: Returns the stored score immediately if the position was already solved.
  2.  **Move Ordering**: Retrieves the "Best Move" from previous shallow searches to sort the move list, maximizing pruning efficiency.

### 🛡️ 3. Heuristic Evaluation

The evaluator prioritizes "Unbreakable" wins and defensive stability:

- **Safety Filtering**: The AI identifies "Dead Stones" (stones that will be captured next turn) and excludes them from pattern formation. It does not chase "fake" wins.
- **Defensive Bias**: The opponent's potential score is weighted heavily (x1.2). This makes the AI "paranoid" and prioritizes blocking threats over creating equal threats.
- **Static Analysis**: Evaluates patterns like `Open Four`, `Closed Four`, and `Split Three` using bitmasks.
