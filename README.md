🎵 C++ Console Audio Visualizer

A little project I'm building while learning C++.

This is a real-time audio visualizer for Windows that captures system audio and turns it into animated ASCII bars in the console. Just experimenting with audio processing, Windows APIs, and making terminal output a bit more interesting.

Features

Real-time audio capture using WASAPI loopback

Animated ASCII bars with randomized characters

Adjustable audio sensitivity and bar smoothing

BPM estimation and tempo-based color cycling

Live elapsed-time and memory-usage display

Requirements

Windows

Microsoft Visual C++ compiler (cl.exe)

Windows SDK

Build

Open a Visual Studio Developer Command Prompt and run:

cl /EHsc visualizer.cpp Psapi.lib

Run

visualizer.exe

Play some audio through your system output and watch the bars react.

What I'm Learning

C++ fundamentals and debugging

Windows API and console handling

WASAPI loopback audio capture

Audio level analysis and smoothing

Basic beat detection and BPM estimation

Real-time console rendering

Things to Improve

FFT-based frequency analysis

Separate bass, mids, and treble visualization

Better BPM detection

More visual effects and customization

This is a learning project, so things might be a little rough around the edges. That's part of the fun.
