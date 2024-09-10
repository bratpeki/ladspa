# ampdb.c

A decibel-based amplifier.

Features
**"Peki's dB Amplifier (Mono)"** *[2001]* and
**"Peki's dB Amplifier (Stereo)"** *[2002]*.

# softclip.c

A `y = tanh(5x)` soft clipper, taken from the "Harmonic Instability of Digital Soft Clipping
Algorithms" paper by Sean Enderby and Zlatko Baracskai
[(link)](https://dafx12.york.ac.uk/papers/dafx12_submission_45.pdf).

The clipper features and adjustable dB threshold which works by amplifying the signal by the given
value, applying the threshold, the lowering by the same threshold volume.

Features
**"Peki's Soft Clipper (Mono)"** *[2003]* and
**"Peki's Soft Clipper (Stereo)"** *[2004]*.
