# Wheel Speed Sensors

The 2024-2025 vehicle will be integrating sprag clutches into each of the four wheels. This allows for all four wheels to be fully locked together when power is applied, but all four wheels to be fully unlocked while coasting. There are numerous benefits to this setup that are not relevant to this project, however they present the need for individual wheel speed sensors.

These sensors provide data to the suspension subteam about how each wheel is rotating as we move through corners. Along with other vehicle data such as yaw rate, GPS position, vehicle velocity, and steering angle, suspension can get a better picture of how these sprag clutches affect vehicle performance. 

## Potential Designs

With the sprag clutches, the wheels, wheel hubs, and brake rotor all spin together. However, this rotational speed can differ from that of the axles when the clutches are in use. Thus, the sensor must detect either wheel, wheel hub, or rotor rotation.

### Gear Tooth/Ferrous Metal Target Sensor (chosen design)

These type of sensors (like the gear tooth speed sensors we have in stock -- [datasheet here](https://www.jameco.com/Jameco/Products/ProdDS/512401.pdf)) provide a digital output that tracks ferrous metal targets. Their datasheet states that they can be used for detecting holes in rotating discs, such as a brake rotor. Using the brake rotor is ideal for this situation as it does not require adding any additional hardware (such as magnets or a trigger wheel), and it actually reduces the weight of the rotor.

The ones we have in stock are no longer for sale, but [these GS100701](https://www.digikey.com/en/products/detail/zf-electronics/GS100701/280932) could work.

It seems best fit to put four holes in the edge of the rotor that will be detected by this sensor. This sensor can be mounted to a tab/bracket on the caliper, and its distance to the rotor can be adjusted using the sensor's threads/nuts. Because the rotor/caliper distances never change, we can get the sensor close enough to the rotor for accurate readings. 

Adding more than 1 hole in the edge of the rotor allows for more accurate readings. At slower speeds, there will only be a few readings per second which does not paint the most accurate picture of the true wheel speed. More than four holes could be added if the sensor/microcontroller is capable of higher polling rates.

### Permanent magnets and hall effects

If one or more permanent magnets are attached to the wheel, we can use a hall effect to detect when this magnetic field passed by. However, this means we must find a way to securely attach the magnets so that they do not fall off while spinning. Magnets also run the risk of being damaged

### Lasers/Infrared

These could be used to detect when a white stripe passes by the emitter/receiver pair (like the CVT tachometer), but dirt, mud, and other substances can cover the sensors or the sensing target.

# Suspension Travel Sensors

The 2024-2025 vehicle will also be integrating four linear suspension travel sensors. These will give an analog reading which represents the linear position of the shocks. We can then use this data to extrapolate the displacement of the wheels. We are using the LPPS-22-200 sensors from HGSI which have a 200mm stroke. They are essentially just a linear potentiometer, so they can be used with a variety of supply voltages.

# Electrical Design

These eight sensors (four wheel speed and four suspension travel) can be run to a single microcontroller located inside the vehicle, which can then transmit the data over CAN. Ideally, each wheel speed sensor reading is interrupt-based on the microcontroller so that calculations for RPM can be done as fast as possible. Suspension travel are analog so we will simply poll those for values Each sensor (at the enclosure side) has 3 pin aviation plug connectors. This prevents them from accidentally being connected to the general power/CAN bus while still maintaing our aviation plug standard. At the sensor side, each sensor uses a different, smaller in-line 3 pin connector so that the sensors themselves can be removed and replaced without removing the entire wiring harness.
