/*
  apDespill Plugin for Nuke
  ------------------------------
  Copyright (c) 2025 Gonzalo Rojas
  This plugin is free to use, modify, and distribute.
  Provided "as is" without any warranty.
*/

#ifndef DESPILL_AP_H
#define DESPILL_AP_H

#include "DDImage/Iop.h"
#include "DDImage/NukeWrapper.h"
using namespace DD::Image;
#include "DDImage/Row.h"
#include "DDImage/Tile.h"
#include "DDImage/Knobs.h"
#include "DDImage/Format.h"

#define HELP "DespillAP v1.0\n"                                                                                                                                                                               \
             "\n"                                                                                                                                                                                             \
             "DespillAP is a native Nuke node designed to remove color spill from images with precision and efficiency.\n"                                                                                    \
             "\n"                                                                                                                                                                                             \
             "Based on the algorithms and conceptual design of Adrian Pueyo's apDespill, DespillAP incorporates advanced features to provide both creative and technical control over the despill process.\n" \
             "\n"                                                                                                                                                                                             \
             "Despill Color knob : selects the color you want to remove from the image.\n"                                                                                                                    \
             "Absolute Mode knob : performs a despill operation toward a specific color or emulates a key, similar to tools like Keylight.\n"                                                                 \
             "Image Inputs       : allows connection of image inputs to define the despill color, respill color, or limits for a fully image-based despill workflow.\n"                                       \
             "Tone Protection     : preserves key tones in the image during the despill process.\n"                                                                                                           \
             "\n"                                                                                                                                                                                             \
             "Tip: Default settings are optimized to avoid extra calculations, providing quick and effective results.\n"                                                                                      \
             "\n"                                                                                                                                                                                             \
             "Copyright 2025. Developed by Gonzalo Rojas.\n";

static const char *const CLASS = "DespillAP";

class DespillAPIop : public Iop
{
public:
    // constructor
    DespillAPIop(Node *node);

    int minimum_inputs() const { return 1; }
    int maximum_inputs() const { return 4; }

    void knobs(Knob_Callback f);
    int knob_changed(Knob *k);

    void _validate(bool);

    void _request(int x, int y, int r, int t, ChannelMask channels, int count);

    void engine(int y, int l, int r, ChannelMask channels, Row &row);

    const char *input_label(int n, char *) const;
    static const Iop::Description d;

    const char *Class() const { return d.name; }
    const char *node_help() const { return HELP; }

private:
    bool d_imgBased;
    bool d_absMode;
    int d_colorType;
    int d_outputType;
    int d_despillMath;
    int d_respillMath;
    float d_spillPick;
    float d_respillColor;
    bool d_outputAlpha;
    bool d_invertAlpha;
    float d_customMath;
    float d_hueOffset;
    float d_hueLimit;
    bool d_protectTones;
    bool d_protectPrev;
    float d_protectColor;
    float d_protectTolerance;
    float d_protectFalloff;
    float d_protectEffect;
    ChannelSet d_defaultChannels;
};

#endif // DESPILL_AP_H