// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Standard/typical colors from http://www.codeproject.com/KB/GDI/XHtmlDraw/XHtmlDraw4.png
// to/fromstrings whose values match the below will evaluate to be the below. i.e. fromstring
// recognizes the names of the colors and tostring will yield a named color if the values
// match.
#pragma once
#ifndef oBase_colors_h
#define oBase_colors_h

#include <oBase/color.h>

namespace ouro {

static const color alice_blue(0xFFF0F8FF);
static const color almost_black(0xFF252525); // useful for a renderer's clear color so black doesn't hide unlit draws
static const color antique_white(0xFFFAEBD7);
static const color aqua(0xFF00FFFF);
static const color aquamarine(0xFF7FFFD4);
static const color azure(0xFFF0FFFF);
static const color beige(0xFFF5F5DC);
static const color bisque(0xFFFFE4C4);
static const color black(0xFF000000);
static const color blanched_almond(0xFFFFEBCD);
static const color blue(0xFF0000FF);
static const color blue_violet(0xFF8A2BE2);
static const color brown(0xFFA52A2A);
static const color burly_wood(0xFFDEB887);
static const color cadet_blue(0xFF5F9EA0);
static const color chartreuse(0xFF7FFF00);
static const color chocolate(0xFFD2691E);
static const color coral(0xFFFF7F50);
static const color cornflower_blue(0xFF6495ED);
static const color cornsilk(0xFFFFF8DC);
static const color crimson(0xFFDC143C);
static const color cyan(0xFF00FFFF);
static const color cark_blue(0xFF00008B);
static const color dark_cyan(0xFF008B8B);
static const color dark_goldenrod(0xFFB8860B);
static const color dark_gray(0xFFA9A9A9);
static const color dark_green(0xFF006400);
static const color dark_khaki(0xFFBDB76B);
static const color dark_magenta(0xFF8B008B);
static const color dark_olive_green(0xFF556B2F);
static const color dark_orange(0xFFFF8C00);
static const color dark_orchid(0xFF9932CC);
static const color dark_red(0xFF8B0000);
static const color dark_salmon(0xFFE9967A);
static const color dark_sea_green(0xFF8FBC8F);
static const color dark_slate_blue(0xFF483D8B);
static const color dark_slate_gray(0xFF2F4F4F);
static const color dark_turquoise(0xFF00CED1);
static const color dark_violet(0xFF9400D3);
static const color deep_pink(0xFFFF1493);
static const color deep_sky_blue(0xFF00BFFF);
static const color dim_gray(0xFF696969);
static const color dodger_blue(0xFF1E90FF);
static const color fire_brick(0xFFB22222);
static const color floral_white(0xFFFFFAF0);
static const color forest_green(0xFF228B22);
static const color fuchsia(0xFFFF00FF);
static const color gainsboro(0xFFDCDCDC);
static const color ghost_white(0xFFF8F8FF);
static const color gold(0xFFFFD700);
static const color goldenRod(0xFFDAA520);
static const color gray(0xFF808080);
static const color green(0xFF008000);
static const color green_yellow(0xFFADFF2F);
static const color honey_dew(0xFFF0FFF0);
static const color hot_pink(0xFFFF69B4);
static const color indian_red (0xFFCD5C5C);
static const color indigo (0xFF4B0082);
static const color ivory(0xFFFFFFF0);
static const color khaki(0xFFF0E68C);
static const color lavender(0xFFE6E6FA);
static const color lavender_lush(0xFFFFF0F5);
static const color lawn_green(0xFF7CFC00);
static const color lemon_chiffon(0xFFFFFACD);
static const color light_blue(0xFFADD8E6);
static const color light_coral(0xFFF08080);
static const color light_cyan(0xFFE0FFFF);
static const color light_goldenrod_yellow(0xFFFAFAD2);
static const color light_gray(0xFFD3D3D3);
static const color light_green(0xFF90EE90);
static const color light_pink(0xFFFFB6C1);
static const color light_salmon(0xFFFFA07A);
static const color light_sea_green(0xFF20B2AA);
static const color light_sky_blue(0xFF87CEFA);
static const color light_slate_gray(0xFF778899);
static const color light_steel_blue(0xFFB0C4DE);
static const color light_yellow(0xFFFFFFE0);
static const color lime(0xFF00FF00);
static const color lime_green(0xFF32CD32);
static const color linen(0xFFFAF0E6);
static const color magenta(0xFFFF00FF);
static const color maroon(0xFF800000);
static const color mask_red(oColorMaskRed);
static const color mask_green(oColorMaskGreen);
static const color mask_blue(oColorMaskBlue);
static const color mask_alpha(oColorMaskAlpha);
static const color medium_aquamarine(0xFF66CDAA);
static const color medium_blue(0xFF0000CD);
static const color medium_orchid(0xFFBA55D3);
static const color medium_purple(0xFF9370D8);
static const color medium_sea_green(0xFF3CB371);
static const color medium_slate_blue(0xFF7B68EE);
static const color medium_spring_green(0xFF00FA9A);
static const color medium_turquoise(0xFF48D1CC);
static const color medium_violet_red(0xFFC71585);
static const color midnight_blue(0xFF191970);
static const color mint_cream(0xFFF5FFFA);
static const color misty_rose(0xFFFFE4E1);
static const color moccasin(0xFFFFE4B5);
static const color navajo_white(0xFFFFDEAD);
static const color navy(0xFF000080);
static const color old_lace(0xFFFDF5E6);
static const color olive(0xFF808000);
static const color olive_drab(0xFF6B8E23);
static const color orange(0xFFFFA500);
static const color orange_red(0xFFFF4500);
static const color orchid(0xFFDA70D6);
static const color pale_goldenrod(0xFFEEE8AA);
static const color pale_green(0xFF98FB98);
static const color pale_turquoise(0xFFAFEEEE);
static const color pale_violet_red(0xFFD87093);
static const color papaya_whip(0xFFFFEFD5);
static const color peach_puff(0xFFFFDAB9);
static const color peru(0xFFCD853F);
static const color pink(0xFFFFC0CB);
static const color plum(0xFFDDA0DD);
static const color powder_blue(0xFFB0E0E6);
static const color purple(0xFF800080);
static const color red(0xFFFF0000);
static const color rosy_brown(0xFFBC8F8F);
static const color royal_blue(0xFF4169E1);
static const color saddle_brown(0xFF8B4513);
static const color salmon(0xFFFA8072);
static const color sandy_brown(0xFFF4A460);
static const color sea_green(0xFF2E8B57);
static const color sea_shell(0xFFFFF5EE);
static const color sienna(0xFFA0522D);
static const color silver(0xFFC0C0C0);
static const color sky_blue(0xFF87CEEB);
static const color slate_blue(0xFF6A5ACD);
static const color slate_gray(0xFF708090);
static const color snow(0xFFFFFAFA);
static const color springGreen(0xFF00FF7F);
static const color steelBlue(0xFF4682B4);
static const color tan_(0xFFD2B48C); // "tan" can conflict with tangent
static const color teal(0xFF008080);
static const color thistle(0xFFD8BFD8);
static const color tomato(0xFFFF6347);
static const color turquoise(0xFF40E0D0);
static const color violet(0xFFEE82EE);
static const color wheat(0xFFF5DEB3);
static const color white(0xFFFFFFFF);
static const color white_smoke(0xFFF5F5F5);
static const color yellow(0xFFFFFF00);
static const color yellow_green(0xFF9ACD32);
static const color microsoft_blue_pen(0xFFFF3232);
static const color microsoft_blue_brush(0xFFA08064);
static const color tangent_space_normal_blue(0xFF7F7FFF); // Z-Up
static const color object_space_normal_green(0xFF7FFF7F); // y-Up

}

#endif
