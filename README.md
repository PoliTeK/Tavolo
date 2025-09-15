# Tavolo
Tavolo is a drone synthesizer that asks everyone to play it. 
It removes the entry barrier through the use of different sensors that enables the user to play using their own body and interacting with the objects on the table instead of using a keyboard.

The user interface consist in:
- Two Cubes : each of them has 2 sliders for the notes of the voices ad 2 infrared distance sensor that act as a filter for each of the total 4 voices of the instrument. The voices are 1 octave apart.
- Pression Sensor : Paired with the lower voices, the harder you press it the more the bass is distorted.
- RM modulation table : Two switches paired with two knobs enables the RM modulation of the higher voices , the knobs add a Delta on top the carrier frequency (the carrier frequencies have an offset of one 5th over the fundamental note played in that moment)
- Joystick : Controls FX choosed at the moment on Ableton Live
- Visuals : X-Y Rapresentation of the sound on Cathodic Rays Televisions
- Plant : Capacitive measurement used as triggers to make a plant on the table play

Tavolo was the first project of PoliteK. It was Imagined as an interactive instrument for events and installations.

Entirely Programmed via the mozzi Library on STM32-F401RE using Arduino Arduino's C++.
