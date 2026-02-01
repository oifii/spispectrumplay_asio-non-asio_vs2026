## Project Files and Layout – Licensing and Documentation Files

This section details the key **documentation** and **licensing** files that define project provenance, usage rights, and context within the larger AUDIO_SPI toolkit.

### README.txt 📄

The `README.txt` file introduces the AUDIO_SPI collection and positions this application within that ecosystem. It provides users and contributors with:

- **Overview of AUDIO_SPI**

Explains that AUDIO_SPI is a set of open-source audio applications for Windows, leveraging popular libraries for easy portability.

- **Project History**

Notes that Stéphane Poirier began the AUDIO_SPI project in Montreal in 2010 to explore software-based music creation.

- **Usage Context**

Mentions that composer Carl Poirier uses some AUDIO_SPI apps to prototype the XAOS audio operating system.

- **Support & Resources**

Provides links for documentation, website, and community support.

```text
README for the AUDIO_SPI applications
=====================================
AUDIO_SPI is a set of open source audio applications targeted for the Windows Operating System.
This application is part of the AUDIO_SPI software collection.
AUDIO_SPI's applications are based on popular open source libraries
and should therefore be fairly easy to port to various platforms.
AUDIO_SPI project was started in Montreal in 2010 by Stephane Poirier
with the objective of experimenting with audio to understand the process of
creating music with software applications.
Some AUDIO_SPI applications are used by Carl Poirier who is a Montreal
music composer prototyping an audio operating system called XAOS.
The packaging of the AUDIO_SPI applications sometimes reflects the XAOS requirements.

Links:
======
The AUDIO_SPI web site  . . . . . https://audiospi.com
Support . . . . . . . . . . https://groups.google.com/forum/#!forum/audio_spi-users
```

### License Notice in `spispectrumplay.cpp` 📜

The top of **spispectrumplay.cpp** declares the project’s license terms and acknowledges upstream examples. Key elements include:

- **Copyright**

Declares ownership by Stéphane Poirier (2012–2026).

- **License Grant**

States that the program is distributed under the GNU GPL v3 (or later), allowing free use, modification, and redistribution.

- **Warranty Disclaimer**

Explicitly disclaims all warranties, including merchantability and fitness for purpose.

- **Upstream Acknowledgment**

Credits the original BASS spectrum-analyzer example from Un4seen Developments.

```c
/*
 * Copyright (c) 2012-2026 Stéphane Poirier
 * stephane.poirier@oifii.org
 * 1901 rue Gilford, #53
 * Montreal, QC, H2H 1G8
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* BASS spectrum analyser example
   Copyright (c) 2002-2012 Un4seen Developments Ltd. */
```

---

**Why These Files Matter**

- They ensure **legal clarity** for users and developers.
- They document the project’s **heritage** within the AUDIO_SPI suite.
- They guide **portability** by highlighting use of open-source libraries.