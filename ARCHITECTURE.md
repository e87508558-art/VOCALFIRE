# VOCALFIRE architecture

Audio graph (target):
Input -> input trim -> pitch detector -> scale quantizer -> pitch shifter/formant stage
-> EQ -> compressor -> de-esser -> saturation -> send FX (reverb/delay) -> output.

Realtime rules:
- No allocations, locks, file I/O or UI calls in processBlock.
- Parameters exposed via AudioProcessorValueTreeState.
- Pitch analysis runs on a bounded frame/window and publishes metering atomically.
- Future pitch correction must report latency and smooth target-note changes.
- UI communicates only through APVTS/atomics; DSP owns audio state.

Alpha implementation:
- JUCE VST3/Standalone project.
- Basic input/output gain, high-pass, delay, reverb.
- Autocorrelation-based F0 estimate for UI meter.
- Pitch Amount/Key/Scale are reserved controls, not yet connected to pitch shifting.
