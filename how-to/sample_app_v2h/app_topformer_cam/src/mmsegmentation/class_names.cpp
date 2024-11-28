/*******************************************************************************
* Copyright 2020 The MMSegmentation Authors. All rights reserved.
* ADE20K class definition `g_ade_classes` and segmentation color palette `g_ade_palette` 
* are made by referring to the following file in Open-MMLab MMSegmentation.
* File : https://github.com/open-mmlab/mmsegmentation/blob/main/mmseg/utils/class_names.py
* Please refer to the `LICENSE.txt` located in `mmsegmentation` directory along with this file.
*******************************************************************************/
#include <stdint.h>
#include <string>

/*******************************************************************************
* Global Tables
*******************************************************************************/
const std::string g_ade_classes[] =
{
    "wall", 
    "building", 
    "sky", 
    "floor", 
    "tree", 
    "ceiling", 
    "road", 
    "bed ",
    "windowpane", 
    "grass", 
    "cabinet", 
    "sidewalk", 
    "person", 
    "earth",
    "door", 
    "table",
    "mountain",
    "plant",
    "curtain",
    "chair",
    "car",
    "water",
    "painting",
    "sofa",
    "shelf",
    "house",
    "sea",
    "mirror",
    "rug",
    "field",
    "armchair",
    "seat",
    "fence",
    "desk",
    "rock",
    "wardrobe",
    "lamp",
    "bathtub",
    "railing",
    "cushion",
    "base",
    "box",
    "column",
    "signboard",
    "chest of drawers",
    "counter",
    "sand",
    "sink",
    "skyscraper",
    "fireplace",
    "refrigerator",
    "grandstand",
    "path",
    "stairs",
    "runway",
    "case",
    "pool table",
    "pillow",
    "screen door",
    "stairway",
    "river",
    "bridge",
    "bookcase",
    "blind",
    "coffee table",
    "toilet",
    "flower",
    "book",
    "hill",
    "bench",
    "countertop",
    "stove",
    "palm",
    "kitchen island",
    "computer",
    "swivel chair",
    "boat",
    "bar",
    "arcade machine",
    "hovel",
    "bus",
    "towel",
    "light",
    "truck",
    "tower",
    "chandelier",
    "awning",
    "streetlight",
    "booth",
    "television receiver",
    "airplane",
    "dirt track",
    "apparel",
    "pole",
    "land",
    "bannister",
    "escalator",
    "ottoman",
    "bottle",
    "buffet",
    "poster",
    "stage",
    "van",
    "ship",
    "fountain",
    "conveyer belt",
    "canopy",
    "washer",
    "plaything",
    "swimming pool",
    "stool",
    "barrel",
    "basket",
    "waterfall",
    "tent",
    "bag",
    "minibike",
    "cradle",
    "oven",
    "ball",
    "food",
    "step",
    "tank",
    "trade name",
    "microwave",
    "pot",
    "animal",
    "bicycle",
    "lake",
    "dishwasher",
    "screen",
    "blanket",
    "sculpture",
    "hood",
    "sconce",
    "vase",
    "traffic light",
    "tray",
    "ashcan",
    "fan",
    "pier",
    "crt screen",
    "plate",
    "monitor",
    "bulletin board",
    "shower",
    "radiator",
    "glass",
    "clock",
    "flag"
};

uint8_t g_ade_palette[][3] = 
{
    {120, 120, 120},  // "wall",       
    {180, 120, 120},  // "building",   
    {6, 230, 230},    // "sky",        
    {80, 50, 50},     // "floor",      
    {4, 200, 3},      // "tree",       
    {120, 120, 80},   // "ceiling",    
    {140, 140, 140},  // "road",       
    {204, 5, 255},    // "bed ",       
    {230, 230, 230},  // "windowpane", 
    {4, 250, 7},      // "grass",      
    {224, 5, 255},    // "cabinet",    
    {235, 255, 7},    // "sidewalk",   
    {150, 5, 61},     // "person",     
    {120, 120, 70},   // "earth",      
    {8, 255, 51},     // "door",       
    {255, 6, 82},     // "table",      
    {143, 255, 140},  // "mountain",   
    {204, 255, 4},    // "plant",      
    {255, 51, 7},     // "curtain",    
    {204, 70, 3},     // "chair",      
    {0, 102, 200},    // "car",        
    {61, 230, 250},   // "water",      
    {255, 6, 51},     // "painting",   
    {11, 102, 255},   // "sofa",       
    {255, 7, 71},     // "shelf",      
    {255, 9, 224},    // "house",      
    {9, 7, 230},      // "sea",        
    {220, 220, 220},  // "mirror",     
    {255, 9, 92},     // "rug",        
    {112, 9, 255},    // "field",      
    {8, 255, 214},    // "armchair",   
    {7, 255, 224},    // "seat",       
    {255, 184, 6},    // "fence",      
    {10, 255, 71},    // "desk",       
    {255, 41, 10},    // "rock",       
    {7, 255, 255},    // "wardrobe",   
    {224, 255, 8},    // "lamp",       
    {102, 8, 255},    // "bathtub",    
    {255, 61, 6},     // "railing",    
    {255, 194, 7},    // "cushion",    
    {255, 122, 8},    // "base",       
    {0, 255, 20},     // "box",        
    {255, 8, 41},     // "column",     
    {255, 5, 153},    // "signboard",  
    {6, 51, 255},     // "chest of draw
    {235, 12, 255},   // "counter",    
    {160, 150, 20},   // "sand",       
    {0, 163, 255},    // "sink",       
    {140, 140, 140},  // "skyscraper", 
    {250, 10, 15},    // "fireplace",  
    {20, 255, 0},     // "refrigerator"
    {31, 255, 0},     // "grandstand", 
    {255, 31, 0},     // "path",       
    {255, 224, 0},    // "stairs",     
    {153, 255, 0},    // "runway",     
    {0, 0, 255},      // "case",       
    {255, 71, 0},     // "pool table", 
    {0, 235, 255},    // "pillow",     
    {0, 173, 255},    // "screen door",
    {31, 0, 255},     // "stairway",   
    {11, 200, 200},   // "river",      
    {255, 82, 0},     // "bridge",     
    {0, 255, 245},    // "bookcase",   
    {0, 61, 255},     // "blind",      
    {0, 255, 112},    // "coffee table"
    {0, 255, 133},    // "toilet",     
    {255, 0, 0},      // "flower",     
    {255, 163, 0},    // "book",       
    {255, 102, 0},    // "hill",       
    {194, 255, 0},    // "bench",      
    {0, 143, 255},    // "countertop", 
    {51, 255, 0},     // "stove",      
    {0, 82, 255},     // "palm",       
    {0, 255, 41},     // "kitchen islan
    {0, 255, 173},    // "computer",   
    {10, 0, 255},     // "swivel chair"
    {173, 255, 0},    // "boat",       
    {0, 255, 153},    // "bar",        
    {255, 92, 0},     // "arcade machin
    {255, 0, 255},    // "hovel",      
    {255, 0, 245},    // "bus",        
    {255, 0, 102},    // "towel",      
    {255, 173, 0},    // "light",      
    {255, 0, 20},     // "truck",      
    {255, 184, 184},  // "tower",      
    {0, 31, 255},     // "chandelier", 
    {0, 255, 61},     // "awning",     
    {0, 71, 255},     // "streetlight",
    {255, 0, 204},    // "booth",      
    {0, 255, 194},    // "television re
    {0, 255, 82},     // "airplane",   
    {0, 10, 255},     // "dirt track", 
    {0, 112, 255},    // "apparel",    
    {51, 0, 255},     // "pole",       
    {0, 194, 255},    // "land",       
    {0, 122, 255},    // "bannister",  
    {0, 255, 163},    // "escalator",  
    {255, 153, 0},    // "ottoman",    
    {0, 255, 10},     // "bottle",     
    {255, 112, 0},    // "buffet",     
    {143, 255, 0},    // "poster",     
    {82, 0, 255},     // "stage",      
    {163, 255, 0},    // "van",        
    {255, 235, 0},    // "ship",       
    {8, 184, 170},    // "fountain",   
    {133, 0, 255},    // "conveyer belt
    {0, 255, 92},     // "canopy",     
    {184, 0, 255},    // "washer",     
    {255, 0, 31},     // "plaything",  
    {0, 184, 255},    // "swimming pool
    {0, 214, 255},    // "stool",      
    {255, 0, 112},    // "barrel",     
    {92, 255, 0},     // "basket",     
    {0, 224, 255},    // "waterfall",  
    {112, 224, 255},  // "tent",       
    {70, 184, 160},   // "bag",        
    {163, 0, 255},    // "minibike",   
    {153, 0, 255},    // "cradle",     
    {71, 255, 0},     // "oven",       
    {255, 0, 163},    // "ball",       
    {255, 204, 0},    // "food",       
    {255, 0, 143},    // "step",       
    {0, 255, 235},    // "tank",       
    {133, 255, 0},    // "trade name", 
    {255, 0, 235},    // "microwave",  
    {245, 0, 255},    // "pot",        
    {255, 0, 122},    // "animal",     
    {255, 245, 0},    // "bicycle",    
    {10, 190, 212},   // "lake",       
    {214, 255, 0},    // "dishwasher", 
    {0, 204, 255},    // "screen",     
    {20, 0, 255},     // "blanket",    
    {255, 255, 0},    // "sculpture",  
    {0, 153, 255},    // "hood",       
    {0, 41, 255},     // "sconce",     
    {0, 255, 204},    // "vase",       
    {41, 0, 255},     // "traffic light
    {41, 255, 0},     // "tray",       
    {173, 0, 255},    // "ashcan",     
    {0, 245, 255},    // "fan",        
    {71, 0, 255},     // "pier",       
    {122, 0, 255},    // "crt screen", 
    {0, 255, 184},    // "plate",      
    {0, 92, 255},     // "monitor",    
    {184, 255, 0},    // "bulletin boar
    {0, 133, 255},    // "shower",     
    {255, 214, 0},    // "radiator",   
    {25, 194, 194},   // "glass",      
    {102, 255, 0},    // "clock",      
    {92, 0, 255}      // "flag"
};