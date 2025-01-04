/*
  apDespill Plugin for Nuke
  ------------------------------
  Copyright (c) 2024 Gonzalo Rojas
  This plugin is free to use, modify, and distribute.
  Provided "as is" without any warranty.
*/

#include "DDImage/Iop.h"
#include "DDImage/NukeWrapper.h"
using namespace DD::Image;
#include "DDImage/Row.h"
#include "DDImage/Tile.h"
#include "DDImage/Knobs.h"
#include "DDImage/Format.h"

static const char *const CLASS = "DespillAP";
static const char *const HELP = "";

class DespillAPIop : public Iop
{
public:
    //constructor
    DespillAPIop(Node *node);

    int minimum_inputs() const { return 1; }
	int maximum_inputs() const { return 4; }

    void knobs(Knob_Callback f);
    int knob_changed(Knob* k);

    void _validate(bool);

    void _request(int x, int y, int r, int t, ChannelMask channels, int count);

    void engine(int y, int l, int r, ChannelMask channels, Row& row);

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