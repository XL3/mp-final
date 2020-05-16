---
duedate: June 2nd
geometry:
- margin=1in
monofont: inconsolata
---
# Final Assessment

## Light Panel
### 10x10 LED grid
- 4b x 4b   output  (row, column) enable
- 1b        output  address enable
- 1b        output  diagnostic mode flag

### 10-key keypad
- 4b        input   key detection
- 1b        input   activity flag

### Audio out device
- 1b        analog-output   sound control

----------------------------------------------------------------------------------------------------

## Cake Decorator
### Making the cake
- Open the eggs valve        (500  ms): `servo motor`
- Start the mixer {10 rev/s} (500  ms): `stepper motor`
- Open the vanilla valve     (100  ms)
- Keep rotating the mixer    (400  ms), then stop
- Open the sugar valve       (200  ms)
- Rotate the mixer           (1000 ms), then reduce speed to half (5 rev/s)
- Open the flour valve       (100  ms)
- Rotate the mixer           (800  ms)

### Baking the cake
- Move the production line one full rotation: `stepper motor`
- Light the oven (1000 ms): `red LED`
- Move the production line again one full rotation
- Stop (1000 ms)

### Decorating the cake
- Input decoration from user: `keypad`
- Decorate the cake
    + Rotate cake stand: `stepper motor`
    + Move decorating arm: `servo motor`, `red LED`, `yellow LED`


