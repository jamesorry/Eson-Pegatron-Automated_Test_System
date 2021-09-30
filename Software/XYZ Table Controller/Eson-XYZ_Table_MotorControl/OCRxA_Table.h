#ifndef _OCRAXA_TABLE_H_
#define _OCRAXA_TABLE_H_


#define OCRAXA_TABLE_FREQ_MAX	25024
#define OCRAXA_TABLE_FREQ_BASE	256
#define OCRAXA_TABLE_INC_EXP	6		//2^6 = 64
#define OCRAXA_TABLE_PRESCALER	1

//Prescaler:1, 增量:64, 最低頻率:256
//0, 1, 2, 3, 4, 5, 6, 7, 8, 9

//const PROGMEM unsigned int OCRxA_Table[] = {	//記憶體不足時用PROGMEM
const unsigned int OCRxA_Table[] = {
		62500, 50000, 41666, 35714, 31250, 27777, 25000, 22727, 20833, 19230,   //, 256, 0
		17857, 16666, 15625, 14705, 13888, 13157, 12500, 11904, 11363, 10869,   //, 896, 1
		10416, 10000, 9615, 9259, 8928, 8620, 8333, 8064, 7812, 7575,   //, 1536, 2
		7352, 7142, 6944, 6756, 6578, 6410, 6250, 6097, 5952, 5813,   //, 2176, 3
		5681, 5555, 5434, 5319, 5208, 5102, 5000, 4901, 4807, 4716,   //, 2816, 4
		4629, 4545, 4464, 4385, 4310, 4237, 4166, 4098, 4032, 3968,   //, 3456, 5
		3906, 3846, 3787, 3731, 3676, 3623, 3571, 3521, 3472, 3424,   //, 4096, 6
		3378, 3333, 3289, 3246, 3205, 3164, 3125, 3086, 3048, 3012,   //, 4736, 7
		2976, 2941, 2906, 2873, 2840, 2808, 2777, 2747, 2717, 2688,   //, 5376, 8
		2659, 2631, 2604, 2577, 2551, 2525, 2500, 2475, 2450, 2427,   //, 6016, 9
		2403, 2380, 2358, 2336, 2314, 2293, 2272, 2252, 2232, 2212,   //, 6656, 10
		2192, 2173, 2155, 2136, 2118, 2100, 2083, 2066, 2049, 2032,   //, 7296, 11
		2016, 2000, 1984, 1968, 1953, 1937, 1923, 1908, 1893, 1879,   //, 7936, 12
		1865, 1851, 1838, 1824, 1811, 1798, 1785, 1773, 1760, 1748,   //, 8576, 13
		1736, 1724, 1712, 1700, 1689, 1677, 1666, 1655, 1644, 1633,   //, 9216, 14
		1623, 1612, 1602, 1592, 1582, 1572, 1562, 1552, 1543, 1533,   //, 9856, 15
		1524, 1515, 1506, 1497, 1488, 1479, 1470, 1461, 1453, 1445,   //, 10496, 16
		1436, 1428, 1420, 1412, 1404, 1396, 1388, 1381, 1373, 1366,   //, 11136, 17
		1358, 1351, 1344, 1336, 1329, 1322, 1315, 1308, 1302, 1295,   //, 11776, 18
		1288, 1282, 1275, 1269, 1262, 1256, 1250, 1243, 1237, 1231,   //, 12416, 19
		1225, 1219, 1213, 1207, 1201, 1196, 1190, 1184, 1179, 1173,   //, 13056, 20
		1168, 1162, 1157, 1152, 1146, 1141, 1136, 1131, 1126, 1121,   //, 13696, 21
		1116, 1111, 1106, 1101, 1096, 1091, 1086, 1082, 1077, 1072,   //, 14336, 22
		1068, 1063, 1059, 1054, 1050, 1046, 1041, 1037, 1033, 1028,   //, 14976, 23
		1024, 1020, 1016, 1012, 1008, 1004, 1000, 996, 992, 988,   //, 15616, 24
		984, 980, 976, 972, 968, 965, 961, 957, 954, 950,   //, 16256, 25
		946, 943, 939, 936, 932, 929, 925, 922, 919, 915,   //, 16896, 26
		912, 909, 905, 902, 899, 896, 892, 889, 886, 883,   //, 17536, 27
		880, 877, 874, 871, 868, 865, 862, 859, 856, 853,   //, 18176, 28
		850, 847, 844, 841, 838, 836, 833, 830, 827, 825,   //, 18816, 29
		822, 819, 816, 814, 811, 809, 806, 803, 801, 798,   //, 19456, 30
		796, 793, 791, 788, 786, 783, 781, 778, 776, 773,   //, 20096, 31
		771, 769, 766, 764, 762, 759, 757, 755, 753, 750,   //, 20736, 32
		748, 746, 744, 741, 739, 737, 735, 733, 730, 728,   //, 21376, 33
		726, 724, 722, 720, 718, 716, 714, 712, 710, 708,   //, 22016, 34
		706, 704, 702, 700, 698, 696, 694, 692, 690, 688,   //, 22656, 35
		686, 684, 683, 681, 679, 677, 675, 673, 672, 670,   //, 23296, 36
		668, 666, 664, 663, 661, 659, 657, 656, 654, 652,   //, 23936, 37
		651, 649, 647, 645, 644, 642, 641, 639, 637, 636,   //, 24576, 38 
#if 0		
		634, 632, 631, 629, 628, 626, 625, 623, 621, 620,   //, 25216, 39
		618, 617, 615, 614, 612, 611, 609, 608, 606, 605,   //, 25856, 40
		603, 602, 600, 599, 598, 596, 595, 593, 592, 591,   //, 26496, 41
		589, 588, 586, 585, 584, 582, 581, 580, 578, 577,   //, 27136, 42
		576, 574, 573, 572, 570, 569, 568, 566, 565, 564,   //, 27776, 43
		563, 561, 560, 559, 558, 556, 555, 554, 553, 551,   //, 28416, 44
		550, 549, 548, 547, 545, 544, 543, 542, 541, 539,       //, 29056, 45
		538, 537, 536, 535, 534, 533, 531, 530, 529, 528,   // 29696, 46
#endif		
};

#define ACCELERATE_TABLE_FREQ_STARTUP_MIN	1000
#define ACCELERATE_TABLE_EXP		10		
const unsigned int AccelerateTable[] = {
		435, 739, 1164, 1446, 1679, 
		1873, 2027, 2160, 2283, 2385, 
		2488, 2560, 2662, 2713, 2795, 
		2867, 2918, 2990, 3051, 3102, 
		3153, 3443, 3589, 3758, 3953, 
		4177
};

#endif //_OCRAXA_TABLE_H_
