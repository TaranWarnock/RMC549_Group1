"""
===============================================================================
file:   link_margin_down_majortom.py
author: Taran Warnock
date:   July 6, 2018
-------------------------------------------------------------------------------
PH 549 Major Tom Link Margin
-------------------------------------------------------------------------------
Calculations of link margin for LoRa.
===============================================================================
Reference: https://electronics.stackexchange.com/questions/277722/how-to-calculate-the-data-rate-of-lora
    How to determine the data rate for the LoRa module. The defaults in the
    initialization of the sensor are:
        - bandwidth, Bw = 125 kHz
        - coding rate, Cr = 4/5
        - spreading factor, Sf = 128 chips/symbol

Reference: http://www.zdacomm.com/400-mhz-uhf-antenna/400-mhz-uhf-omni-directional-antenna/
    Omnidirectional antennas can have gain as low as 2 dB. We will calculate
    the link margin for this worst case scenario.
"""

import numpy as np
from scipy.constants import speed_of_light

if __name__ == '__main__':
    # Step 1: Determine the Effective Isotropic Radiated Power (EIRP)
    transmit_power_W = 0.2  # corresponds to 23 dBm as set in arduino code
    transmit_power_dB = 10.0 * np.log10(transmit_power_W)
    line_loss_dB = 1.0
    transmit_gain_dB = 10 * np.log10(1.64)  # typical dipole gain
    effective_isotropic_radiated_power_dB = transmit_power_dB + transmit_gain_dB - line_loss_dB

    # Step 2: Determine the space loss, L_s, due to propagation of the signal through space
    frequency_Hz = 434.0e6
    wavelength_m = speed_of_light / frequency_Hz
    earth_radius_m = 6378000.0
    satellite_height_m = 30000.0
    minimum_elevation_angle_deg = 1.0
    minimum_elevation_angle_rad = np.deg2rad(minimum_elevation_angle_deg)
    distance_m = -earth_radius_m * np.sin(minimum_elevation_angle_rad) + np.sqrt(earth_radius_m ** 2 * (
        np.sin(minimum_elevation_angle_rad)) ** 2 + 2 * satellite_height_m * earth_radius_m + satellite_height_m ** 2)
    space_loss = (4 * np.pi * distance_m / wavelength_m) ** 2
    space_loss_dB = 10.0 * np.log10(space_loss)

    # Step 3: Determine Values
    receiver_gain_dB = 2.0  # omnidirectional antenna
    receiver_system_noise_K = 600.0     # using typical value from PH 547 assignment
    system_noise_dB = 10.0 * np.log10(receiver_system_noise_K)
    data_rate_bps = 5468.75
    data_rate_dB = 10.0 * np.log10(data_rate_bps)
    zenith_attenuation_dB = 2.0e-2
    attenuation_dB = zenith_attenuation_dB / np.sin(minimum_elevation_angle_rad)

    # Step 4: Determine the ratio of received energy per bit to noise-density ratio, E_b/N_o
    received_energy_per_bit_to_noise_density_ratio_dB = \
        effective_isotropic_radiated_power_dB + receiver_gain_dB + 228.6 - system_noise_dB - data_rate_dB - \
        space_loss_dB - attenuation_dB

    # Step 5: Determine the dBs required for the chosen modulation scheme at an acceptable bit error rate (BER)
    required_energy_per_bit_to_noise_density_ratio_dB = 11.5

    # Step 6: Determine system losses that can lead to signal degradation
    polarization_mismatch_loss_dB = 0.3
    antenna_pointing_offset_loss_dB = 0.2
    system_implementation_loss_dB = 1.0

    # Step 7: Calculate the link margin in dBs
    link_margin_dB = received_energy_per_bit_to_noise_density_ratio_dB - \
                     required_energy_per_bit_to_noise_density_ratio_dB - polarization_mismatch_loss_dB - \
                     antenna_pointing_offset_loss_dB - system_implementation_loss_dB
    print('LoRa downlink link margin: {} dB'.format(link_margin_dB))
