/*
  apDespill Plugin for Nuke
  ------------------------------
  Copyright (c) 2025 Gonzalo Rojas
  This plugin is free to use, modify, and distribute.
  Provided "as is" without any warranty.
*/

#include "include/DespillAP.h"

#include "include/Attribute.h"
#include "include/Color.h"
#include "include/Constants.h"
#include "include/Dispatcher.h"
#include "include/Image.h"
#include "include/Utils.h"

enum inputs {
  inputSource = 0,
  inputLimit = 1,
  inputColor = 2,
  inputRespill = 3,
};

DespillAPIop::DespillAPIop(Node *node) : Iop(node)
{
  inputs(3);
  k_limitChannel = Chan_Alpha;
  k_outputSpillChannel = Chan_Alpha;
  k_absMode = 0;
  k_imgBased = 0;
  k_colorType = 3;
  k_spillPick = 0.0f;
  k_respillColor = 1.0f;
  k_outputType = 0;
  k_outputAlpha = 1;
  k_invertAlpha = 1;
  k_despillMath = 0;
  k_customMath = 0.0f;
  k_hueOffset = 0.0f;
  k_hueLimit = 1.0f;
  k_respillMath = 0;
  k_protectColor = 0.0f;
  k_protectTolerance = 0.2f;
  k_protectFalloff = 2.0f;
  k_protectEffect = 1.0f;
  k_invertLimitMask = 1;
}

void DespillAPIop::knobs(Knob_Callback f)
{
  Enumeration_knob(f, &k_colorType, Constants::COLOR_TYPES, "color");
  Bool_knob(f, &k_imgBased, "imageBased", "Image Based");
  ClearFlags(f, Knob::STARTLINE);
  Bool_knob(f, &k_absMode, "absoluteMode", "Absolute Mode");

  Knob *pick_knob = Color_knob(f, &k_spillPick, "pick");
  ClearFlags(f, Knob::MAGNITUDE | Knob::SLIDER);
  pick_knob->set_value(0.0f, 0);
  pick_knob->set_value(1.0f, 1);
  pick_knob->set_value(0.0f, 2);

  Enumeration_knob(f, &k_despillMath, Constants::DESPILL_MATH_TYPES, "despillMath", "math");
  Float_knob(f, &k_customMath, "customMath", "");
  SetFlags(f, Knob::DISABLED);
  SetRange(f, -1, 1);

  Divider(f, "<b>Hue</b>");
  Float_knob(f, &k_hueOffset, "hueOffset", "offset");
  SetRange(f, -30, 30);
  Float_knob(f, &k_hueLimit, "hueLimit", "limit");
  SetRange(f, 0, 2);
  Input_Channel_knob(f, &k_limitChannel, 1, 1, "limitChannel", "mask");
  Bool_knob(f, &k_invertLimitMask, "invertLimitMask", "invert");
  SetFlags(f, Knob::ENDLINE);
  Bool_knob(f, &k_protectTones, "protectTones", "Protect Tones");
  Bool_knob(f, &k_protectPrev, "protectPreview", "Preview");
  SetFlags(f, Knob::DISABLED);
  ClearFlags(f, Knob::STARTLINE);

  BeginGroup(f, "Protect Tones");
  SetFlags(f, Knob::CLOSED);
  Knob *protectColor_knob = Color_knob(f, &k_protectColor, "protectColor", "color");
  ClearFlags(f, Knob::MAGNITUDE | Knob::SLIDER);
  SetFlags(f, Knob::DISABLED);
  protectColor_knob->set_value(0.0f, 0);
  protectColor_knob->set_value(0.0f, 1);
  protectColor_knob->set_value(0.0f, 2);
  Float_knob(f, &k_protectTolerance, "protectTolerance", "tolerance");
  SetFlags(f, Knob::DISABLED);
  SetRange(f, 0, 1);
  Float_knob(f, &k_protectFalloff, "protectFalloff", "falloff");
  SetFlags(f, Knob::DISABLED);
  SetRange(f, 0, 4);
  Float_knob(f, &k_protectEffect, "protectEffect", "effect");
  SetFlags(f, Knob::DISABLED);
  SetRange(f, 0, 10);
  EndGroup(f);

  Divider(f, "<b>Respill</b>");
  Enumeration_knob(f, &k_respillMath, Constants::RESPILL_MATH_TYPES, "respillMath", "math");
  Knob *respillColor_knob = Color_knob(f, &k_respillColor, "respillColor", "color");
  ClearFlags(f, Knob::MAGNITUDE | Knob::SLIDER);
  respillColor_knob->set_value(1.0f, 0);
  respillColor_knob->set_value(1.0f, 1);
  respillColor_knob->set_value(1.0f, 2);
  SetRange(f, 0, 4);

  Divider(f, "<b>Output</b>");
  Enumeration_knob(f, &k_outputType, Constants::OUTPUT_TYPES, "outputDespill", "output");
  Bool_knob(f, &k_outputAlpha, "outputAlpha", "Output Spill Alpha");
  ClearFlags(f, Knob::STARTLINE);
  Bool_knob(f, &k_invertAlpha, "invertAlpha", "Invert");
  SetFlags(f, Knob::ENDLINE);
  Input_Channel_knob(f, &k_outputSpillChannel, 1, 1, "outputSpillChannel", "channel");
  SetFlags(f, Knob::ENDLINE);
  Spacer(f, 0);
}

int DespillAPIop::knob_changed(Knob *k)
{
  if(k->is("despillMath")) {
    Knob *despillMath_knob = k->knob("despillMath");
    Knob *customMath_knob = k->knob("customMath");
    if(despillMath_knob->get_value() == 3) {
      customMath_knob->enable();
    }
    else {
      customMath_knob->disable();
    }
    return 1;
  }

  if(k->is("color")) {
    Knob *color_knob = k->knob("color");
    Knob *pick_knob = k->knob("pick");
    if(color_knob->get_value() == 3) {
      pick_knob->enable();
    }
    else {
      pick_knob->disable();
    }
    return 1;
  }

  if(k->is("protectTones")) {
    Knob *protectTones_knob = k->knob("protectTones");
    Knob *protectColor_knob = k->knob("protectColor");
    Knob *protectTolerance_knob = k->knob("protectTolerance");
    Knob *protectFalloff_knob = k->knob("protectFalloff");
    Knob *protectEffect_knob = k->knob("protectEffect");
    Knob *protectPreview_knob = k->knob("protectPreview");
    if(protectTones_knob->get_value() == 1) {
      protectColor_knob->enable();
      protectTolerance_knob->enable();
      protectFalloff_knob->enable();
      protectEffect_knob->enable();
      protectPreview_knob->enable();
    }
    else {
      protectColor_knob->disable();
      protectTolerance_knob->disable();
      protectFalloff_knob->disable();
      protectEffect_knob->disable();
      protectPreview_knob->disable();
    }
    return 1;
  }
  knob("tile_color")->set_value(0x8b8b8bff);  // node color
  return 0;
}

const char *DespillAPIop::input_label(int n, char *) const
{
  switch(n) {
    case 0:
      return "Source";
    case 1:
      return "Limit";
    case 2:
      return "Color";
    case 3:
      return "Respill";
    default:
      return 0;
  }
}

void DespillAPIop::set_input(int i, Op *inputOp, int input, int offset)
{
  Iop::set_input(i, inputOp, input, offset);

  if(inputOp) {
    // Check if input color is connected
    if(i == inputColor && inputOp->node_name() != "Black in root") {
      std::cout << "connected! " << "\n";
    }
    else {
      if(i == inputColor) {
        std::cout << "not connected! " << "\n";
      }
    }
  }
}

void DespillAPIop::_validate(bool for_real)
{
  copy_info(0);

  nuke::ChannelSet outChannels = channels();
  outChannels += k_outputSpillChannel;
  set_out_channels(outChannels);
  info_.turn_on(outChannels);
}

void DespillAPIop::_request(int x, int y, int r, int t, ChannelMask channels, int count)
{
  nuke::ChannelSet requestedChannels = channels;
  requestedChannels += Mask_RGB;

  input(inputSource)->request(input(inputSource)->info().box(), requestedChannels, Mask_RGB);

  if(input(inputLimit) != nullptr) {
    input(inputLimit)->request(Mask_Alpha, count);
  };
  if(input(inputColor) != nullptr) {
    input(inputColor)->request(input(inputColor)->info().box(), Mask_RGB, count);
  };
  if(input(inputRespill) != nullptr) {
    input(inputRespill)->request(input(inputRespill)->info().box(), Mask_RGB, count);
  };
}

void DespillAPIop::engine(int y, int x, int r, ChannelMask channels, Row &row)
{
  callCloseAfter(0);
  ProcessCPU(y, x, r, channels, row);
}

void DespillAPIop::ProcessCPU(int y, int x, int r, ChannelMask channels, Row &row)
{
  nuke::ChannelSet requestedChannels = channels;
  requestedChannels += Mask_RGBA;  // Add RGBA
  row.get(input0(), y, x, r, requestedChannels);

  // Copy all other channels
  nuke::ChannelSet copyMask = channels - nuke::Mask_RGBA;
  row.pre_copy(row, copyMask);
  row.copy(row, copyMask, x, r);

  float rgb[3] = {0, 0, 0};
  float spillLuma[3] = {0, 0, 0};
  imgcore::Pixel<const float> inPixel(3);
  imgcore::Pixel<float> outPixel(3);

  for(int i = 0; i < 3; ++i) {
    inPixel.SetPtr(row[static_cast<nuke::Channel>(i + 1)] + x, i);
    outPixel.SetPtr(row.writable(static_cast<nuke::Channel>(i + 1)) + x, i);
  }

  // Loop through the pixels
  for(int x0 = x; x0 < r; ++x0) {
    for(int i = 0; i < 3; i++) {
      rgb[i] = inPixel.GetVal(i);
    }

    float spillMatte = 0.0f;

    // Apply functions to rgb
    color::HueRotate(rgb, rgb, k_hueOffset);

    // Write RGB channels to the output
    for(int i = 0; i < 3; i++) {
      outPixel[i] = rgb[i];
    }

    // Write spill channel
    foreach(z, channels) {
      if(z == k_outputSpillChannel) {
        *(row.writable(z) + x0) = spillMatte;
      }
    }
    inPixel++;
    outPixel++;
  }
}

static Iop *build(Node *node)
{
  return (new NukeWrapper(new DespillAPIop(node)))->noChannels();
}
const Iop::Description DespillAPIop::d("DespillAP", "Keyer/DespillAP", build);