# Quiz Scoring System Using a 3-Digit 7-Segment Display with ESP32
## Introduction
This project is a simulation/miniature of a quiz scoring system controlled by an ESP32. The purpose of this project is to create a simulation of a smart and careful assessment system with a button for each team, with three teams participating so there are three buttons. Each team also has a score indicator with a maximum value of 9, where this indicator will also show the name of the team, namely "A", "B", and "C". In addition, from the judge, there are also buttons, namely a button to state whether the answer is right or wrong and a reset button to go to the next stage.
## Demo Video
https://youtu.be/3s6_UFzeSBg

## System Overview
This system is designed for quiz competitions, featuring:
- **3-digit 7-segment displays** (one unit for each team).
- **3 participant buttons** for teams A, B, and C.
- **3 judge buttons** for control (`Standby`, `Correct`, and `Incorrect`).

## Features
1. **Participant Buttons for Teams**  
   - Three buttons for teams A, B, and C allow participants to compete by pressing them quickly.  
   - The name of the fastest team is displayed for evaluation by the judge.

2. **Initial State and Reset**  
   - When the system is powered on or reset:
     - All participant buttons are disabled.
     - All 7-segment displays show the scores for teams A, B, and C.

3. **Standby Mode**  
   - After the judge presses the `Standby` button:
     - The displays show the team names (`A`, `B`, `C`).
     - Participant buttons are enabled for 15 seconds (to allow question reading and participant responses).  
     - If no button is pressed within 15 seconds, participant buttons are disabled again, returning to the initial state.

4. **Fastest Button Press Handling**  
   - If a button is pressed within 15 seconds:
     - The display shows the name of the fastest team.
     - Other displays are disabled.
     - The judge evaluates the response using the `Correct` or `Incorrect` button.

5. **Correct Answer**  
   - If the judge presses `Correct`:
     - The team's score increases by 1.
     - All team scores are displayed.
     - Only the `Standby` button is enabled to start the next question.

6. **Incorrect Answer**  
   - If the judge presses `Incorrect`:
     - The team's score decreases by 1 and is displayed.
     - The system enters a 3-second standby mode, enabling the other teams to answer the same question.
     - Displays show the names of the remaining teams.

7. **Second Attempt Handling**  
   - If another team presses its button during the standby period:
     - A `Correct` evaluation increases their score by 1, displays all scores, and disables participant buttons until `Standby` is pressed.
     - An `Incorrect` evaluation decreases their score by 1, displays all scores, and only enables the `Standby` button.

8. **Force Reset During Standby**  
   - Pressing the `Standby` button twice within 0.5 seconds:
     - Disables participant buttons.
     - Displays all team scores.
     - The system waits for the `Standby` button to be pressed again.

9. **Automatic End Condition**  
   - The system ends automatically when a team reaches a score of 9:
     - The winning team's score is displayed.
     - Displays for other teams are turned off.
     - The system resets only after the `Standby` button is pressed.

10. **Buzzer Accessory**  
    - A buzzer can be added to indicate status changes.
    - The tone and duration can be customized, considering technical constraints.

## Components 
1. 7-segment Common Anode (You can use a single three-digit seven-segment display instead of three single-digit seven-segment displays.)
2. Micro Limit Switch Normally Closed
3. ESP32
4. Passive Buzzer
5. Transistor NPN BC547
6. Some Resistor
## Schematic
![Schematic](https://github.com/user-attachments/assets/065c8b9c-7b83-4924-8680-05b514561706)
