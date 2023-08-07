/*-------------------------------------------------------------------------
NeoColorFeatures includes all the feature classes that describe color order and
color depth for NeoPixelBus template class

Written by Michael C. Miller.

I invest time and resources providing this open source code,
please support me by dontating (see https://github.com/Makuna/NeoPixelBus)

-------------------------------------------------------------------------
This file is part of the Makuna/NeoPixelBus library.

NeoPixelBus is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

NeoPixelBus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with NeoPixel.  If not, see
<http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------*/
#pragma once

// NeoPixel Features
//
#include "features/Neo2ByteElements.h"
#include "features/Neo3ByteElements.h"
#include "features/Neo4ByteElements.h"
#include "features/Neo6ByteElements.h"
#include "features/Neo6Byte4xxElements.h"
#include "features/Neo8ByteElements.h"
#include "features/NeoBgrFeature.h"
#include "features/NeoBrgFeature.h"
#include "features/NeoGrb48Feature.h"
#include "features/NeoGrbFeature.h"
#include "features/NeoGrbwFeature.h"
#include "features/NeoRbgFeature.h"
#include "features/NeoRgb48Feature.h"
#include "features/NeoRgbFeature.h"
#include "features/NeoRgbw64Feature.h"
#include "features/NeoRgbwFeature.h"
#include "features/NeoRgbwxxFeature.h"
#include "features/NeoSm168xxColorFeatures.h"
#include "features/NeoTm1814ColorFeatures.h"
#include "features/NeoTm1914ColorFeatures.h"

typedef NeoRgb48Feature NeoRgbUcs8903Feature;
typedef NeoRgbw64Feature NeoRgbwUcs8904Feature;
typedef NeoGrb48Feature NeoGrbWs2816Feature;

// DotStart Features
// 
#include "features/DotStar3Elements.h"
#include "features/DotStar4Elements.h"
#include "features/DotStarBgrFeature.h"
#include "features/DotStarBrgFeature.h"
#include "features/DotStarGbrFeature.h"
#include "features/DotStarGrbFeature.h"
#include "features/DotStarLbgrFeature.h"
#include "features/DotStarLbrgFeature.h"
#include "features/DotStarLgbrFeature.h"
#include "features/DotStarLgrbFeature.h"
#include "features/DotStarLrbgFeature.h"
#include "features/DotStarLrgbFeature.h"
#include "features/DotStarRbgFeature.h"
#include "features/DotStarRgbFeature.h"
#include "features/Lpd8806BrgFeature.h"
#include "features/Lpd8806GrbFeature.h"
#include "features/Lpd6803BrgFeature.h"
#include "features/Lpd6803GrbFeature.h"
#include "features/Lpd6803GbrFeature.h"
#include "features/Lpd6803RgbFeature.h"
#include "features/P9813BgrFeature.h"

// 7 Segment Features
//
#include "features/Neo9ByteElements.h"
#include "features/NeoAbcdefgpsSegmentFeature.h"
#include "features/NeoBacedfpgsSegmentFeature.h"

typedef NeoAbcdefgpsSegmentFeature SevenSegmentFeature; // Abcdefg order is default
