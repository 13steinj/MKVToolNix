/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit https://www.gnu.org/licenses/old-licenses/gpl-2.0.html

   ISO 3166 countries

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

// ------------------------------------------------------------------------
// NOTE: this file is auto-generated by the "dev:iso3166_list" rake target.
// ------------------------------------------------------------------------

#include "common/common_pch.h"

#include "common/iso3166.h"

namespace mtx::iso3166 {

std::vector<country_t> const g_countries{
  { "AD"s, "AND"s,  20, u8"Andorra"s,                                      u8"Principality of Andorra"s                               },
  { "AE"s, "ARE"s, 784, u8"United Arab Emirates"s,                         u8""s                                                      },
  { "AF"s, "AFG"s,   4, u8"Afghanistan"s,                                  u8"Islamic Republic of Afghanistan"s                       },
  { "AG"s, "ATG"s,  28, u8"Antigua and Barbuda"s,                          u8""s                                                      },
  { "AI"s, "AIA"s, 660, u8"Anguilla"s,                                     u8""s                                                      },
  { "AL"s, "ALB"s,   8, u8"Albania"s,                                      u8"Republic of Albania"s                                   },
  { "AM"s, "ARM"s,  51, u8"Armenia"s,                                      u8"Republic of Armenia"s                                   },
  { "AO"s, "AGO"s,  24, u8"Angola"s,                                       u8"Republic of Angola"s                                    },
  { "AQ"s, "ATA"s,  10, u8"Antarctica"s,                                   u8""s                                                      },
  { "AR"s, "ARG"s,  32, u8"Argentina"s,                                    u8"Argentine Republic"s                                    },
  { "AS"s, "ASM"s,  16, u8"American Samoa"s,                               u8""s                                                      },
  { "AT"s, "AUT"s,  40, u8"Austria"s,                                      u8"Republic of Austria"s                                   },
  { "AU"s, "AUS"s,  36, u8"Australia"s,                                    u8""s                                                      },
  { "AW"s, "ABW"s, 533, u8"Aruba"s,                                        u8""s                                                      },
  { "AX"s, "ALA"s, 248, u8"Åland Islands"s,                                u8""s                                                      },
  { "AZ"s, "AZE"s,  31, u8"Azerbaijan"s,                                   u8"Republic of Azerbaijan"s                                },
  { "BA"s, "BIH"s,  70, u8"Bosnia and Herzegovina"s,                       u8"Republic of Bosnia and Herzegovina"s                    },
  { "BB"s, "BRB"s,  52, u8"Barbados"s,                                     u8""s                                                      },
  { "BD"s, "BGD"s,  50, u8"Bangladesh"s,                                   u8"People's Republic of Bangladesh"s                       },
  { "BE"s, "BEL"s,  56, u8"Belgium"s,                                      u8"Kingdom of Belgium"s                                    },
  { "BF"s, "BFA"s, 854, u8"Burkina Faso"s,                                 u8""s                                                      },
  { "BG"s, "BGR"s, 100, u8"Bulgaria"s,                                     u8"Republic of Bulgaria"s                                  },
  { "BH"s, "BHR"s,  48, u8"Bahrain"s,                                      u8"Kingdom of Bahrain"s                                    },
  { "BI"s, "BDI"s, 108, u8"Burundi"s,                                      u8"Republic of Burundi"s                                   },
  { "BJ"s, "BEN"s, 204, u8"Benin"s,                                        u8"Republic of Benin"s                                     },
  { "BL"s, "BLM"s, 652, u8"Saint Barthélemy"s,                             u8""s                                                      },
  { "BM"s, "BMU"s,  60, u8"Bermuda"s,                                      u8""s                                                      },
  { "BN"s, "BRN"s,  96, u8"Brunei Darussalam"s,                            u8""s                                                      },
  { "BO"s, "BOL"s,  68, u8"Bolivia, Plurinational State of"s,              u8"Plurinational State of Bolivia"s                        },
  { "BQ"s, "BES"s, 535, u8"Bonaire, Sint Eustatius and Saba"s,             u8"Bonaire, Sint Eustatius and Saba"s                      },
  { "BR"s, "BRA"s,  76, u8"Brazil"s,                                       u8"Federative Republic of Brazil"s                         },
  { "BS"s, "BHS"s,  44, u8"Bahamas"s,                                      u8"Commonwealth of the Bahamas"s                           },
  { "BT"s, "BTN"s,  64, u8"Bhutan"s,                                       u8"Kingdom of Bhutan"s                                     },
  { "BV"s, "BVT"s,  74, u8"Bouvet Island"s,                                u8""s                                                      },
  { "BW"s, "BWA"s,  72, u8"Botswana"s,                                     u8"Republic of Botswana"s                                  },
  { "BY"s, "BLR"s, 112, u8"Belarus"s,                                      u8"Republic of Belarus"s                                   },
  { "BZ"s, "BLZ"s,  84, u8"Belize"s,                                       u8""s                                                      },
  { "CA"s, "CAN"s, 124, u8"Canada"s,                                       u8""s                                                      },
  { "CC"s, "CCK"s, 166, u8"Cocos (Keeling) Islands"s,                      u8""s                                                      },
  { "CD"s, "COD"s, 180, u8"Congo, The Democratic Republic of the"s,        u8""s                                                      },
  { "CF"s, "CAF"s, 140, u8"Central African Republic"s,                     u8""s                                                      },
  { "CG"s, "COG"s, 178, u8"Congo"s,                                        u8"Republic of the Congo"s                                 },
  { "CH"s, "CHE"s, 756, u8"Switzerland"s,                                  u8"Swiss Confederation"s                                   },
  { "CI"s, "CIV"s, 384, u8"Côte d'Ivoire"s,                                u8"Republic of Côte d'Ivoire"s                             },
  { "CK"s, "COK"s, 184, u8"Cook Islands"s,                                 u8""s                                                      },
  { "CL"s, "CHL"s, 152, u8"Chile"s,                                        u8"Republic of Chile"s                                     },
  { "CM"s, "CMR"s, 120, u8"Cameroon"s,                                     u8"Republic of Cameroon"s                                  },
  { "CN"s, "CHN"s, 156, u8"China"s,                                        u8"People's Republic of China"s                            },
  { "CO"s, "COL"s, 170, u8"Colombia"s,                                     u8"Republic of Colombia"s                                  },
  { "CR"s, "CRI"s, 188, u8"Costa Rica"s,                                   u8"Republic of Costa Rica"s                                },
  { "CU"s, "CUB"s, 192, u8"Cuba"s,                                         u8"Republic of Cuba"s                                      },
  { "CV"s, "CPV"s, 132, u8"Cabo Verde"s,                                   u8"Republic of Cabo Verde"s                                },
  { "CW"s, "CUW"s, 531, u8"Curaçao"s,                                      u8"Curaçao"s                                               },
  { "CX"s, "CXR"s, 162, u8"Christmas Island"s,                             u8""s                                                      },
  { "CY"s, "CYP"s, 196, u8"Cyprus"s,                                       u8"Republic of Cyprus"s                                    },
  { "CZ"s, "CZE"s, 203, u8"Czechia"s,                                      u8"Czech Republic"s                                        },
  { "DE"s, "DEU"s, 276, u8"Germany"s,                                      u8"Federal Republic of Germany"s                           },
  { "DJ"s, "DJI"s, 262, u8"Djibouti"s,                                     u8"Republic of Djibouti"s                                  },
  { "DK"s, "DNK"s, 208, u8"Denmark"s,                                      u8"Kingdom of Denmark"s                                    },
  { "DM"s, "DMA"s, 212, u8"Dominica"s,                                     u8"Commonwealth of Dominica"s                              },
  { "DO"s, "DOM"s, 214, u8"Dominican Republic"s,                           u8""s                                                      },
  { "DZ"s, "DZA"s,  12, u8"Algeria"s,                                      u8"People's Democratic Republic of Algeria"s               },
  { "EC"s, "ECU"s, 218, u8"Ecuador"s,                                      u8"Republic of Ecuador"s                                   },
  { "EE"s, "EST"s, 233, u8"Estonia"s,                                      u8"Republic of Estonia"s                                   },
  { "EG"s, "EGY"s, 818, u8"Egypt"s,                                        u8"Arab Republic of Egypt"s                                },
  { "EH"s, "ESH"s, 732, u8"Western Sahara"s,                               u8""s                                                      },
  { "ER"s, "ERI"s, 232, u8"Eritrea"s,                                      u8"the State of Eritrea"s                                  },
  { "ES"s, "ESP"s, 724, u8"Spain"s,                                        u8"Kingdom of Spain"s                                      },
  { "ET"s, "ETH"s, 231, u8"Ethiopia"s,                                     u8"Federal Democratic Republic of Ethiopia"s               },
  { "FI"s, "FIN"s, 246, u8"Finland"s,                                      u8"Republic of Finland"s                                   },
  { "FJ"s, "FJI"s, 242, u8"Fiji"s,                                         u8"Republic of Fiji"s                                      },
  { "FK"s, "FLK"s, 238, u8"Falkland Islands (Malvinas)"s,                  u8""s                                                      },
  { "FM"s, "FSM"s, 583, u8"Micronesia, Federated States of"s,              u8"Federated States of Micronesia"s                        },
  { "FO"s, "FRO"s, 234, u8"Faroe Islands"s,                                u8""s                                                      },
  { "FR"s, "FRA"s, 250, u8"France"s,                                       u8"French Republic"s                                       },
  { "GA"s, "GAB"s, 266, u8"Gabon"s,                                        u8"Gabonese Republic"s                                     },
  { "GB"s, "GBR"s, 826, u8"United Kingdom"s,                               u8"United Kingdom of Great Britain and Northern Ireland"s  },
  { "GD"s, "GRD"s, 308, u8"Grenada"s,                                      u8""s                                                      },
  { "GE"s, "GEO"s, 268, u8"Georgia"s,                                      u8""s                                                      },
  { "GF"s, "GUF"s, 254, u8"French Guiana"s,                                u8""s                                                      },
  { "GG"s, "GGY"s, 831, u8"Guernsey"s,                                     u8""s                                                      },
  { "GH"s, "GHA"s, 288, u8"Ghana"s,                                        u8"Republic of Ghana"s                                     },
  { "GI"s, "GIB"s, 292, u8"Gibraltar"s,                                    u8""s                                                      },
  { "GL"s, "GRL"s, 304, u8"Greenland"s,                                    u8""s                                                      },
  { "GM"s, "GMB"s, 270, u8"Gambia"s,                                       u8"Republic of the Gambia"s                                },
  { "GN"s, "GIN"s, 324, u8"Guinea"s,                                       u8"Republic of Guinea"s                                    },
  { "GP"s, "GLP"s, 312, u8"Guadeloupe"s,                                   u8""s                                                      },
  { "GQ"s, "GNQ"s, 226, u8"Equatorial Guinea"s,                            u8"Republic of Equatorial Guinea"s                         },
  { "GR"s, "GRC"s, 300, u8"Greece"s,                                       u8"Hellenic Republic"s                                     },
  { "GS"s, "SGS"s, 239, u8"South Georgia and the South Sandwich Islands"s, u8""s                                                      },
  { "GT"s, "GTM"s, 320, u8"Guatemala"s,                                    u8"Republic of Guatemala"s                                 },
  { "GU"s, "GUM"s, 316, u8"Guam"s,                                         u8""s                                                      },
  { "GW"s, "GNB"s, 624, u8"Guinea-Bissau"s,                                u8"Republic of Guinea-Bissau"s                             },
  { "GY"s, "GUY"s, 328, u8"Guyana"s,                                       u8"Republic of Guyana"s                                    },
  { "HK"s, "HKG"s, 344, u8"Hong Kong"s,                                    u8"Hong Kong Special Administrative Region of China"s      },
  { "HM"s, "HMD"s, 334, u8"Heard Island and McDonald Islands"s,            u8""s                                                      },
  { "HN"s, "HND"s, 340, u8"Honduras"s,                                     u8"Republic of Honduras"s                                  },
  { "HR"s, "HRV"s, 191, u8"Croatia"s,                                      u8"Republic of Croatia"s                                   },
  { "HT"s, "HTI"s, 332, u8"Haiti"s,                                        u8"Republic of Haiti"s                                     },
  { "HU"s, "HUN"s, 348, u8"Hungary"s,                                      u8"Hungary"s                                               },
  { "ID"s, "IDN"s, 360, u8"Indonesia"s,                                    u8"Republic of Indonesia"s                                 },
  { "IE"s, "IRL"s, 372, u8"Ireland"s,                                      u8""s                                                      },
  { "IL"s, "ISR"s, 376, u8"Israel"s,                                       u8"State of Israel"s                                       },
  { "IM"s, "IMN"s, 833, u8"Isle of Man"s,                                  u8""s                                                      },
  { "IN"s, "IND"s, 356, u8"India"s,                                        u8"Republic of India"s                                     },
  { "IO"s, "IOT"s,  86, u8"British Indian Ocean Territory"s,               u8""s                                                      },
  { "IQ"s, "IRQ"s, 368, u8"Iraq"s,                                         u8"Republic of Iraq"s                                      },
  { "IR"s, "IRN"s, 364, u8"Iran, Islamic Republic of"s,                    u8"Islamic Republic of Iran"s                              },
  { "IS"s, "ISL"s, 352, u8"Iceland"s,                                      u8"Republic of Iceland"s                                   },
  { "IT"s, "ITA"s, 380, u8"Italy"s,                                        u8"Italian Republic"s                                      },
  { "JE"s, "JEY"s, 832, u8"Jersey"s,                                       u8""s                                                      },
  { "JM"s, "JAM"s, 388, u8"Jamaica"s,                                      u8""s                                                      },
  { "JO"s, "JOR"s, 400, u8"Jordan"s,                                       u8"Hashemite Kingdom of Jordan"s                           },
  { "JP"s, "JPN"s, 392, u8"Japan"s,                                        u8""s                                                      },
  { "KE"s, "KEN"s, 404, u8"Kenya"s,                                        u8"Republic of Kenya"s                                     },
  { "KG"s, "KGZ"s, 417, u8"Kyrgyzstan"s,                                   u8"Kyrgyz Republic"s                                       },
  { "KH"s, "KHM"s, 116, u8"Cambodia"s,                                     u8"Kingdom of Cambodia"s                                   },
  { "KI"s, "KIR"s, 296, u8"Kiribati"s,                                     u8"Republic of Kiribati"s                                  },
  { "KM"s, "COM"s, 174, u8"Comoros"s,                                      u8"Union of the Comoros"s                                  },
  { "KN"s, "KNA"s, 659, u8"Saint Kitts and Nevis"s,                        u8""s                                                      },
  { "KP"s, "PRK"s, 408, u8"Korea, Democratic People's Republic of"s,       u8"Democratic People's Republic of Korea"s                 },
  { "KR"s, "KOR"s, 410, u8"Korea, Republic of"s,                           u8""s                                                      },
  { "KW"s, "KWT"s, 414, u8"Kuwait"s,                                       u8"State of Kuwait"s                                       },
  { "KY"s, "CYM"s, 136, u8"Cayman Islands"s,                               u8""s                                                      },
  { "KZ"s, "KAZ"s, 398, u8"Kazakhstan"s,                                   u8"Republic of Kazakhstan"s                                },
  { "LA"s, "LAO"s, 418, u8"Lao People's Democratic Republic"s,             u8""s                                                      },
  { "LB"s, "LBN"s, 422, u8"Lebanon"s,                                      u8"Lebanese Republic"s                                     },
  { "LC"s, "LCA"s, 662, u8"Saint Lucia"s,                                  u8""s                                                      },
  { "LI"s, "LIE"s, 438, u8"Liechtenstein"s,                                u8"Principality of Liechtenstein"s                         },
  { "LK"s, "LKA"s, 144, u8"Sri Lanka"s,                                    u8"Democratic Socialist Republic of Sri Lanka"s            },
  { "LR"s, "LBR"s, 430, u8"Liberia"s,                                      u8"Republic of Liberia"s                                   },
  { "LS"s, "LSO"s, 426, u8"Lesotho"s,                                      u8"Kingdom of Lesotho"s                                    },
  { "LT"s, "LTU"s, 440, u8"Lithuania"s,                                    u8"Republic of Lithuania"s                                 },
  { "LU"s, "LUX"s, 442, u8"Luxembourg"s,                                   u8"Grand Duchy of Luxembourg"s                             },
  { "LV"s, "LVA"s, 428, u8"Latvia"s,                                       u8"Republic of Latvia"s                                    },
  { "LY"s, "LBY"s, 434, u8"Libya"s,                                        u8"Libya"s                                                 },
  { "MA"s, "MAR"s, 504, u8"Morocco"s,                                      u8"Kingdom of Morocco"s                                    },
  { "MC"s, "MCO"s, 492, u8"Monaco"s,                                       u8"Principality of Monaco"s                                },
  { "MD"s, "MDA"s, 498, u8"Moldova, Republic of"s,                         u8"Republic of Moldova"s                                   },
  { "ME"s, "MNE"s, 499, u8"Montenegro"s,                                   u8"Montenegro"s                                            },
  { "MF"s, "MAF"s, 663, u8"Saint Martin (French part)"s,                   u8""s                                                      },
  { "MG"s, "MDG"s, 450, u8"Madagascar"s,                                   u8"Republic of Madagascar"s                                },
  { "MH"s, "MHL"s, 584, u8"Marshall Islands"s,                             u8"Republic of the Marshall Islands"s                      },
  { "MK"s, "MKD"s, 807, u8"North Macedonia"s,                              u8"Republic of North Macedonia"s                           },
  { "ML"s, "MLI"s, 466, u8"Mali"s,                                         u8"Republic of Mali"s                                      },
  { "MM"s, "MMR"s, 104, u8"Myanmar"s,                                      u8"Republic of Myanmar"s                                   },
  { "MN"s, "MNG"s, 496, u8"Mongolia"s,                                     u8""s                                                      },
  { "MO"s, "MAC"s, 446, u8"Macao"s,                                        u8"Macao Special Administrative Region of China"s          },
  { "MP"s, "MNP"s, 580, u8"Northern Mariana Islands"s,                     u8"Commonwealth of the Northern Mariana Islands"s          },
  { "MQ"s, "MTQ"s, 474, u8"Martinique"s,                                   u8""s                                                      },
  { "MR"s, "MRT"s, 478, u8"Mauritania"s,                                   u8"Islamic Republic of Mauritania"s                        },
  { "MS"s, "MSR"s, 500, u8"Montserrat"s,                                   u8""s                                                      },
  { "MT"s, "MLT"s, 470, u8"Malta"s,                                        u8"Republic of Malta"s                                     },
  { "MU"s, "MUS"s, 480, u8"Mauritius"s,                                    u8"Republic of Mauritius"s                                 },
  { "MV"s, "MDV"s, 462, u8"Maldives"s,                                     u8"Republic of Maldives"s                                  },
  { "MW"s, "MWI"s, 454, u8"Malawi"s,                                       u8"Republic of Malawi"s                                    },
  { "MX"s, "MEX"s, 484, u8"Mexico"s,                                       u8"United Mexican States"s                                 },
  { "MY"s, "MYS"s, 458, u8"Malaysia"s,                                     u8""s                                                      },
  { "MZ"s, "MOZ"s, 508, u8"Mozambique"s,                                   u8"Republic of Mozambique"s                                },
  { "NA"s, "NAM"s, 516, u8"Namibia"s,                                      u8"Republic of Namibia"s                                   },
  { "NC"s, "NCL"s, 540, u8"New Caledonia"s,                                u8""s                                                      },
  { "NE"s, "NER"s, 562, u8"Niger"s,                                        u8"Republic of the Niger"s                                 },
  { "NF"s, "NFK"s, 574, u8"Norfolk Island"s,                               u8""s                                                      },
  { "NG"s, "NGA"s, 566, u8"Nigeria"s,                                      u8"Federal Republic of Nigeria"s                           },
  { "NI"s, "NIC"s, 558, u8"Nicaragua"s,                                    u8"Republic of Nicaragua"s                                 },
  { "NL"s, "NLD"s, 528, u8"Netherlands"s,                                  u8"Kingdom of the Netherlands"s                            },
  { "NO"s, "NOR"s, 578, u8"Norway"s,                                       u8"Kingdom of Norway"s                                     },
  { "NP"s, "NPL"s, 524, u8"Nepal"s,                                        u8"Federal Democratic Republic of Nepal"s                  },
  { "NR"s, "NRU"s, 520, u8"Nauru"s,                                        u8"Republic of Nauru"s                                     },
  { "NU"s, "NIU"s, 570, u8"Niue"s,                                         u8"Niue"s                                                  },
  { "NZ"s, "NZL"s, 554, u8"New Zealand"s,                                  u8""s                                                      },
  { "OM"s, "OMN"s, 512, u8"Oman"s,                                         u8"Sultanate of Oman"s                                     },
  { "PA"s, "PAN"s, 591, u8"Panama"s,                                       u8"Republic of Panama"s                                    },
  { "PE"s, "PER"s, 604, u8"Peru"s,                                         u8"Republic of Peru"s                                      },
  { "PF"s, "PYF"s, 258, u8"French Polynesia"s,                             u8""s                                                      },
  { "PG"s, "PNG"s, 598, u8"Papua New Guinea"s,                             u8"Independent State of Papua New Guinea"s                 },
  { "PH"s, "PHL"s, 608, u8"Philippines"s,                                  u8"Republic of the Philippines"s                           },
  { "PK"s, "PAK"s, 586, u8"Pakistan"s,                                     u8"Islamic Republic of Pakistan"s                          },
  { "PL"s, "POL"s, 616, u8"Poland"s,                                       u8"Republic of Poland"s                                    },
  { "PM"s, "SPM"s, 666, u8"Saint Pierre and Miquelon"s,                    u8""s                                                      },
  { "PN"s, "PCN"s, 612, u8"Pitcairn"s,                                     u8""s                                                      },
  { "PR"s, "PRI"s, 630, u8"Puerto Rico"s,                                  u8""s                                                      },
  { "PS"s, "PSE"s, 275, u8"Palestine, State of"s,                          u8"the State of Palestine"s                                },
  { "PT"s, "PRT"s, 620, u8"Portugal"s,                                     u8"Portuguese Republic"s                                   },
  { "PW"s, "PLW"s, 585, u8"Palau"s,                                        u8"Republic of Palau"s                                     },
  { "PY"s, "PRY"s, 600, u8"Paraguay"s,                                     u8"Republic of Paraguay"s                                  },
  { "QA"s, "QAT"s, 634, u8"Qatar"s,                                        u8"State of Qatar"s                                        },
  { "RE"s, "REU"s, 638, u8"Réunion"s,                                      u8""s                                                      },
  { "RO"s, "ROU"s, 642, u8"Romania"s,                                      u8""s                                                      },
  { "RS"s, "SRB"s, 688, u8"Serbia"s,                                       u8"Republic of Serbia"s                                    },
  { "RU"s, "RUS"s, 643, u8"Russian Federation"s,                           u8""s                                                      },
  { "RW"s, "RWA"s, 646, u8"Rwanda"s,                                       u8"Rwandese Republic"s                                     },
  { "SA"s, "SAU"s, 682, u8"Saudi Arabia"s,                                 u8"Kingdom of Saudi Arabia"s                               },
  { "SB"s, "SLB"s,  90, u8"Solomon Islands"s,                              u8""s                                                      },
  { "SC"s, "SYC"s, 690, u8"Seychelles"s,                                   u8"Republic of Seychelles"s                                },
  { "SD"s, "SDN"s, 729, u8"Sudan"s,                                        u8"Republic of the Sudan"s                                 },
  { "SE"s, "SWE"s, 752, u8"Sweden"s,                                       u8"Kingdom of Sweden"s                                     },
  { "SG"s, "SGP"s, 702, u8"Singapore"s,                                    u8"Republic of Singapore"s                                 },
  { "SH"s, "SHN"s, 654, u8"Saint Helena, Ascension and Tristan da Cunha"s, u8""s                                                      },
  { "SI"s, "SVN"s, 705, u8"Slovenia"s,                                     u8"Republic of Slovenia"s                                  },
  { "SJ"s, "SJM"s, 744, u8"Svalbard and Jan Mayen"s,                       u8""s                                                      },
  { "SK"s, "SVK"s, 703, u8"Slovakia"s,                                     u8"Slovak Republic"s                                       },
  { "SL"s, "SLE"s, 694, u8"Sierra Leone"s,                                 u8"Republic of Sierra Leone"s                              },
  { "SM"s, "SMR"s, 674, u8"San Marino"s,                                   u8"Republic of San Marino"s                                },
  { "SN"s, "SEN"s, 686, u8"Senegal"s,                                      u8"Republic of Senegal"s                                   },
  { "SO"s, "SOM"s, 706, u8"Somalia"s,                                      u8"Federal Republic of Somalia"s                           },
  { "SR"s, "SUR"s, 740, u8"Suriname"s,                                     u8"Republic of Suriname"s                                  },
  { "SS"s, "SSD"s, 728, u8"South Sudan"s,                                  u8"Republic of South Sudan"s                               },
  { "ST"s, "STP"s, 678, u8"Sao Tome and Principe"s,                        u8"Democratic Republic of Sao Tome and Principe"s          },
  { "SV"s, "SLV"s, 222, u8"El Salvador"s,                                  u8"Republic of El Salvador"s                               },
  { "SX"s, "SXM"s, 534, u8"Sint Maarten (Dutch part)"s,                    u8"Sint Maarten (Dutch part)"s                             },
  { "SY"s, "SYR"s, 760, u8"Syrian Arab Republic"s,                         u8""s                                                      },
  { "SZ"s, "SWZ"s, 748, u8"Eswatini"s,                                     u8"Kingdom of Eswatini"s                                   },
  { "TC"s, "TCA"s, 796, u8"Turks and Caicos Islands"s,                     u8""s                                                      },
  { "TD"s, "TCD"s, 148, u8"Chad"s,                                         u8"Republic of Chad"s                                      },
  { "TF"s, "ATF"s, 260, u8"French Southern Territories"s,                  u8""s                                                      },
  { "TG"s, "TGO"s, 768, u8"Togo"s,                                         u8"Togolese Republic"s                                     },
  { "TH"s, "THA"s, 764, u8"Thailand"s,                                     u8"Kingdom of Thailand"s                                   },
  { "TJ"s, "TJK"s, 762, u8"Tajikistan"s,                                   u8"Republic of Tajikistan"s                                },
  { "TK"s, "TKL"s, 772, u8"Tokelau"s,                                      u8""s                                                      },
  { "TL"s, "TLS"s, 626, u8"Timor-Leste"s,                                  u8"Democratic Republic of Timor-Leste"s                    },
  { "TM"s, "TKM"s, 795, u8"Turkmenistan"s,                                 u8""s                                                      },
  { "TN"s, "TUN"s, 788, u8"Tunisia"s,                                      u8"Republic of Tunisia"s                                   },
  { "TO"s, "TON"s, 776, u8"Tonga"s,                                        u8"Kingdom of Tonga"s                                      },
  { "TR"s, "TUR"s, 792, u8"Turkey"s,                                       u8"Republic of Turkey"s                                    },
  { "TT"s, "TTO"s, 780, u8"Trinidad and Tobago"s,                          u8"Republic of Trinidad and Tobago"s                       },
  { "TV"s, "TUV"s, 798, u8"Tuvalu"s,                                       u8""s                                                      },
  { "TW"s, "TWN"s, 158, u8"Taiwan, Province of China"s,                    u8"Taiwan, Province of China"s                             },
  { "TZ"s, "TZA"s, 834, u8"Tanzania, United Republic of"s,                 u8"United Republic of Tanzania"s                           },
  { "UA"s, "UKR"s, 804, u8"Ukraine"s,                                      u8""s                                                      },
  { "UG"s, "UGA"s, 800, u8"Uganda"s,                                       u8"Republic of Uganda"s                                    },
  { "UM"s, "UMI"s, 581, u8"United States Minor Outlying Islands"s,         u8""s                                                      },
  { "US"s, "USA"s, 840, u8"United States"s,                                u8"United States of America"s                              },
  { "UY"s, "URY"s, 858, u8"Uruguay"s,                                      u8"Eastern Republic of Uruguay"s                           },
  { "UZ"s, "UZB"s, 860, u8"Uzbekistan"s,                                   u8"Republic of Uzbekistan"s                                },
  { "VA"s, "VAT"s, 336, u8"Holy See (Vatican City State)"s,                u8""s                                                      },
  { "VC"s, "VCT"s, 670, u8"Saint Vincent and the Grenadines"s,             u8""s                                                      },
  { "VE"s, "VEN"s, 862, u8"Venezuela, Bolivarian Republic of"s,            u8"Bolivarian Republic of Venezuela"s                      },
  { "VG"s, "VGB"s,  92, u8"Virgin Islands, British"s,                      u8"British Virgin Islands"s                                },
  { "VI"s, "VIR"s, 850, u8"Virgin Islands, U.S."s,                         u8"Virgin Islands of the United States"s                   },
  { "VN"s, "VNM"s, 704, u8"Viet Nam"s,                                     u8"Socialist Republic of Viet Nam"s                        },
  { "VU"s, "VUT"s, 548, u8"Vanuatu"s,                                      u8"Republic of Vanuatu"s                                   },
  { "WF"s, "WLF"s, 876, u8"Wallis and Futuna"s,                            u8""s                                                      },
  { "WS"s, "WSM"s, 882, u8"Samoa"s,                                        u8"Independent State of Samoa"s                            },
  { "YE"s, "YEM"s, 887, u8"Yemen"s,                                        u8"Republic of Yemen"s                                     },
  { "YT"s, "MYT"s, 175, u8"Mayotte"s,                                      u8""s                                                      },
  { "ZA"s, "ZAF"s, 710, u8"South Africa"s,                                 u8"Republic of South Africa"s                              },
  { "ZM"s, "ZMB"s, 894, u8"Zambia"s,                                       u8"Republic of Zambia"s                                    },
  { "ZW"s, "ZWE"s, 716, u8"Zimbabwe"s,                                     u8"Republic of Zimbabwe"s                                  },
};

} // namespace mtx::iso3166
