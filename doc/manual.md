# ampdb.c

A decibel-based amplifier.

Features
**"Peki's dB Amplifier (Mono)"** *[2001]* and
**"Peki's dB Amplifier (Stereo)"** *[2002]*.

# softclip.c

A `y = tanh(param * x)` soft clipper, taken from the "Harmonic Instability of Digital Soft Clipping
Algorithms" paper by Sean Enderby and Zlatko Baracskai
[(link)](https://dafx12.york.ac.uk/papers/dafx12_submission_45.pdf).

The clipper features an adjustable dB threshold and parameter.
The dB threshold works by amplifying the signal by the given value, applying the clipping,
and then lowering the output by the same threshold value.

The adjustable parameter is the one seen in the mathematical formula given at the start.

I've made an interactive curve that explains this plugin.
It can be seen [here](https://www.desmos.com/calculator/s6djw5g35s).

Features
**"Peki's Soft Clipper (Mono)"** *[2003]* and
**"Peki's Soft Clipper (Stereo)"** *[2004]*.
