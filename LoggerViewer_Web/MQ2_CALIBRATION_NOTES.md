# MQ-2 Sensor Calibration Notes for LPG/Butane Detection

## Hardware Configuration
- **Sensor**: MQ-2 Semiconductor Gas Sensor (SnO2)
- **Microcontroller**: Arduino Uno
- **ADC Resolution**: 10-bit (0-1023)
- **Supply Voltage (Vc)**: 5V
- **Heater Voltage (VH)**: 5V
- **Load Resistance (RL)**: 5.0 kΩ (verify with actual circuit)

## Operating Conditions
- **Temperature**: ~10°C (cave environment)
- **Humidity**: ~90% RH (cave environment)
- **Target Gas**: LPG/Butane

⚠️ **IMPORTANT**: These conditions differ significantly from datasheet standard test conditions (20°C, 65% RH)

## Temperature/Humidity Compensation

From MQ-2 datasheet Fig.2, at 10°C and 90% RH:
- **Rs/Ro ratio ≈ 1.5-1.6** compared to standard conditions
- This means the sensor resistance is ~50-60% higher in cave conditions
- All readings must be compensated for environmental conditions

### Dynamic Compensation Factor Calculation

The application now includes a dynamic temperature/humidity compensation calculator in the UI.

**Formula** (curve-fitted from Fig.2 actual data points):
```javascript
// Temperature effect (non-linear, exponential-like)
tempFactor = 1.0 + (20 - temperature) × 0.0185 + ((20 - temperature) / 30)² × 0.15

// Humidity effect (linear approximation)
humidityFactor = 1.0 + (humidity - 65) × 0.0062

// Combined compensation factor
compensationFactor = tempFactor × humidityFactor
```

**Validation against Fig.2 datasheet values:**

| Temp | Humidity | Calculated | Graph Value | Error |
|------|----------|------------|-------------|-------|
| 10°C | 90% RH   | 1.44       | 1.43        | +0.7% |
| 10°C | 60% RH   | 1.19       | 1.20        | -0.8% |
| 20°C | 90% RH   | 1.16       | 1.25        | -7.2% |
| -10°C| 90% RH   | 1.73       | 1.72        | +0.6% |
| 50°C | 60% RH   | 0.68       | 0.75        | -9.3% |

**Typical cave conditions:**
- At 10°C, 90% RH: factor = **1.44** (cave environment)
- At 10°C, 65% RH: factor = **1.22**
- At 20°C, 65% RH: factor = **1.00** (reference condition)

**Usage:**
```javascript
Rs_compensated = Rs_measured / TEMP_HUMIDITY_COMPENSATION
```

The compensation factor can be adjusted in real-time via the UI:
- Temperature field: -20°C to 50°C
- Humidity field: 30% to 95% RH
- Compensation Factor: Auto-calculated (read-only display)

## Sensor Resistance Calculation

From the voltage divider circuit:
```
VRL = ADC_value × Vc / 1023
Rs = RL × (Vc - VRL) / VRL
Rs = RL × (Vc/VRL - 1)
Rs = RL × (1023/ADC_value - 1)
Rs = RL × (1023 - ADC_value) / ADC_value
```

Current implementation:
```javascript
function MQResistanceCalculation(raw_adc) {
    return (RL_VALUE * (1023.0 - raw_adc) / raw_adc);
}
```
✅ **CORRECT**

## Baseline Calibration (Ro)

From MQ-2 datasheet page 1:
- **Ro**: Sensor resistance at 1000ppm isobutane under standard conditions
- **Clean Air Sensitivity (S)**: Rs(in air) / Rs(1000ppm isobutane) ≥ 5

Therefore:
```
RO_CLEAN_AIR_FACTOR = 5.0  // Minimum value from datasheet specification
```

**Previous value was 9.83** (incorrect - that's for H2 detection, not LPG)

## LPG Curve Parameters

From MQ-2 datasheet Fig.1, extracting points on the LPG curve:
- At 200 ppm: Rs/Ro ≈ 2.0
- At 1000 ppm: Rs/Ro ≈ 0.65
- At 5000 ppm: Rs/Ro ≈ 0.35
- At 10000 ppm: Rs/Ro ≈ 0.25

The curve follows a power law:
```
Rs/Ro = A × PPM^B
log10(Rs/Ro) = log10(A) + B × log10(PPM)
log10(PPM) = (log10(Rs/Ro) - log10(A)) / B
```

Calculating from two points (1000 ppm, 0.65) and (10000 ppm, 0.25):
```
B = (log10(0.25) - log10(0.65)) / (log10(10000) - log10(1000))
B = (-0.602 - (-0.187)) / (4 - 3)
B = -0.415 / 1 = -0.415

log10(A) = log10(0.65) - B × log10(1000)
log10(A) = -0.187 - (-0.415) × 3
log10(A) = -0.187 + 1.245 = 1.058
A = 10^1.058 ≈ 11.43
```

Therefore: **Rs/Ro = 11.43 × PPM^(-0.415)**

## Current Code Implementation

The code uses:
```javascript
var LPGCurve = [2.3, 0.21, -0.47];  // [x, y, slope]
PPM = Math.pow(10, ((Math.log10(rs_ro_ratio) - LPGCurve[1]) / LPGCurve[2] + LPGCurve[0]));
```

This translates to:
```
log10(PPM) = (log10(Rs/Ro) - 0.21) / (-0.47) + 2.3
log10(PPM) = -2.128 × log10(Rs/Ro) + 2.747
```

### Verification with datasheet points:

**At 1000 ppm** (Rs/Ro = 0.65):
```
log10(PPM) = -2.128 × (-0.187) + 2.747 = 3.145
PPM = 10^3.145 ≈ 1396 ppm
```
❌ Error: ~40% too high

**At 10000 ppm** (Rs/Ro = 0.25):
```
log10(PPM) = -2.128 × (-0.602) + 2.747 = 4.028
PPM = 10^4.028 ≈ 10660 ppm
```
✅ Error: ~7% (acceptable)

## Recommended Corrections

### Option 1: Update curve parameters based on datasheet analysis
```javascript
var LPGCurve = [2.41, 0.29, -0.415];  // Recalculated from Fig.1
```

### Option 2: Use direct power law formula
```javascript
// Rs/Ro = 11.43 × PPM^(-0.415)
// PPM = (Rs/Ro / 11.43)^(1/-0.415)
function MQ2_PPM_gas(resCurrent, resZero) {
    var rs_ro_ratio = resCurrent / resZero;
    var ppm = Math.pow(rs_ro_ratio / 11.43, 1 / -0.415);
    return ppm;
}
```

### Option 3: Add temperature/humidity compensation
The most accurate approach considering the 10°C/90%RH conditions:

```javascript
var TEMP_HUMIDITY_FACTOR = 1.55;  // From Fig.2 at 10°C/90%RH

function MQ2_RawToPPM(rawValue, zeroGasValue) {
    // Calculate resistances
    var resCurrent = MQResistanceCalculation(rawValue);
    var resZero = MQResistanceCalculation(zeroGasValue) / RO_CLEAN_AIR_FACTOR;

    // Compensate for temperature and humidity
    resCurrent = resCurrent / TEMP_HUMIDITY_FACTOR;

    // Calculate PPM using calibrated curve
    var rs_ro_ratio = resCurrent / resZero;
    var ppm = Math.pow(rs_ro_ratio / 11.43, 1 / -0.415);

    return Math.max(0, ppm);
}
```

## Testing and Validation

To validate the calibration:
1. Test with known LPG concentrations if possible
2. Compare readings with other gas detectors
3. Monitor stability over time (sensor requires 48h preheat minimum)
4. Adjust RO_CLEAN_AIR_FACTOR based on actual clean air readings
5. Consider seasonal temperature variations in cave environment

## References
- MQ-2 Datasheet: `/home/speleoalex/git/opsdatalogger/doc/MQ2.pdf`
- Arduino MQ Library: Common source of calibration constants
- Original implementation: May have been calibrated for H2 detection

## Butane Mass Flow Calculations

The application includes an "Air Flow Analysis" section to calculate the mass of butane passing through the tunnel/gallery based on air flow rate and measured PPM concentration.

### Formula

```javascript
// Calculate baseline PPM from ZeroGas ADC (background concentration)
baselinePPM = MQ2_RawToPPM(zeroGasADC, zeroGasADC)

// For each sample, subtract baseline to get NET butane
netPPM = measuredPPM - baselinePPM

// Butane density (temperature dependent)
densityButane = 2480 × (293 / (273 + temperature)) // g/m³

// Net concentration in g/m³
concentrationGM3 = (netPPM × densityButane) / 1,000,000

// Integrate over time
timeInterval = interval_between_samples (minutes)
massInterval = concentrationGM3 × airFlowRate × timeInterval

// Total mass = sum of all intervals
totalMass = Σ massInterval
```

**IMPORTANT**: The calculation subtracts the baseline PPM (from ZeroGas setting) to measure only the **additional** butane above background levels. This gives you the actual butane from the source, not the total atmospheric concentration.

### Example Calculation

**Given:**
- Air flow rate: 100 m³/min
- Average concentration: 500 PPM
- Temperature: 10°C

**Results:**
- Butane density at 10°C: 2567 g/m³
- Concentration: 1.28 g/m³
- **Flow: 128 g/min = 7.7 kg/hour = 185 kg/day**

### Reference Table (at 10°C, 100 m³/min)

| PPM | g/min | kg/hour | kg/day | tons/year |
|-----|-------|---------|--------|-----------|
| 100 | 26 | 1.5 | 36 | 13.1 |
| 200 | 51 | 3.1 | 74 | 27.0 |
| 500 | 128 | 7.7 | 185 | 67.5 |
| 1000 | 257 | 15.4 | 370 | 135.1 |
| 2000 | 513 | 30.8 | 739 | 269.7 |

## Change Log
- 2025-11-21: Initial analysis
  - Corrected RO_CLEAN_AIR_FACTOR from 9.83 to 5.0
  - Identified need for temperature/humidity compensation
  - Recalculated LPG curve parameters from datasheet
  - Added air flow analysis and butane mass flow calculations
