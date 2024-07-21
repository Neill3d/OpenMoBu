# GPU Particles Shader

Particles system that is evaluated on gpu.

## Overview

New Elements with the plugin

* **GPU Particles shader** - main shader
* **Collision Sphere marker** - a sphere collision object for particle system
* **Collision Terrain marker** - a terrain generator and collision object
* **Force Drag** - get force from this object velocity
* **Force Wind** - specified direct force (can be turbulence)
* **Force Motor** - radial force field

GPU Shader Particles is located in Assets Templates \ Shading Elements \ Shaders

Collisions and Forces are located in Assets Templates \ Physical Properties \ Particles

![](../../Plugins/Images/gpuParticles\_image1.jpg)

## GPU Shader Particles Properties

### Generation

Emitter - source of particle emitting, could be

* **Volume** - generate a new particle inside the emitter bounding box
* **Vertices** - generate a new particle from one of emitter vertices
* **Surface** - generate a new particle on one of emitter surface polygon

Play Mode

* **Live** - simulate particles in real-time with system time
* **Play** - simulate particles along the timeline with local time

**Use Custom Range** - flag to use specified start/stop local time range, during which particles will be emitted **Emit Start** - specify a local start time of particles emitting **Emit Stop** - specify a local stop time of particles emitting

**Reset Time** - this is an option for local Play Mode. When you enter a reset time, the particles will be reseted to start position **Reset Quantity** - number of particles that will be generated from the beginning on simulation or on a reset operation **Use Rate** - flag to generate number of particles specified in Particle Rate per second **Particle Rate** - how many new particles will be emitted per second

**Use PreGenerated Particles** - very important option. Particles generation could be precached (start random position, start velocity, start color, etc.) or everthing could be calculated on fly

**Emit Direction (unit vector)** - vector direction in local emitter coords system. Particles will use the direction to compute start velocity **Dir Spread Latitude (%)** - particle variation in latitude sperical coord **Dir Spread Longitude (%)** - particle variation in longitude sperical coord

**Use Emitter Normals As dir** - that is options for Vertices and Surface emit type. A new particle will use vertex normal or polygon normal as it's start direction instead of "Emit Direction" property

**Emit Speed (units)** - start velocity speed in "Emit Direction" **Emit Speed Spread (%)** - variation in start velocity speed

**Inherit Emitter Speed** - when flag is on, then particle will inherit velocity from it's emitter in additional to it's own assigned start velocity

**Live Time (secs)** - particle life time in seconds **Live Time Variation (%)** - percentage of particle life time variation

### Dynamic

**Mass** - how heavy the particle is, that is used in applying forces on a particle. 100 is equal to 1 mass unit **Damping** - percentage or damping when applying a velocity. The current particle velocity is slowind down by damping factor.

**Use Gravity** - when flag is on, then particle will be affected by global gravity force F = m \* g2 **Gravity** - global gravity force direction and value

**Use Floor** - when flag is on, particle will collide with a floor plane Floor Friction - Floor Level -

**Use Forces** - when on, particles will be affected by a list of attached forces **Forces** - attached forces objects **Use Collisions** - when on, particles will be affected by a list of attached collisions **Collisions** - attached collision objects

**Self Collisions** - experimental flag, when particles collide with each other with their own assigned radius (size)

**Use Turbulence** - this is a global flow force, noise 3d function that changes particle velocity in time. Effect looks simular to a wind force.
