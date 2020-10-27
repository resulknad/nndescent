# Presentation
- Algorithm [2min] (Jonas)
-- motivation [n^2 vs n^1.14]
-- main bottleneck (random access pattern)
-- only present intuition of algorithm relevant for presentation

[4min]
- Optimizations for **Random Access Pattern**
-- Sampling
-- naive -> pynndescent -> turbosampling [one pass]
-- Motivation: Improvements by locality (sorted MNIST, clustered dataset)
-- copy vs order
-- cachegrind
-- heuristic to reorder data in memory

[4min]
- transition: dimension higher, random access pattern impact lower 
- Higher dimensional :
-- sorting first
-- blocking [cache gets full], [upper triangle]

- Std/General/Trivia:
-- memory alignment
