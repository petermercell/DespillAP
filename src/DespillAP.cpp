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
  k_spillPick[0] = 0.0f;
  k_spillPick[1] = 1.0f;
  k_spillPick[2] = 0.0f;
  k_colorType = 3;
  k_absMode = 0;
  k_respillColor[0] = 1.0f;
  k_respillColor[1] = 1.0f;
  k_respillColor[2] = 1.0f;
  k_outputType = 0;
  k_outputAlpha = 1;
  k_invertAlpha = 1;
  k_despillMath = 0;
  k_customWeight = 0.0f;
  k_hueOffset = 0.0f;
  k_hueLimit = 1.0f;
  k_respillMath = 0;
  k_protectColor[0] = 0.0f;
  k_protectColor[1] = 0.0f;
  k_protectColor[2] = 0.0f;
  k_protectTolerance = 0.2f;
  k_protectFalloff = 2.0f;
  k_protectEffect = 1.0f;
  k_invertLimitMask = 1;

  isSourceConnected = false;
  isLimitConnected = false;
  isColorConnected = false;
  isRespillConnected = false;

  _returnColor = 0;
}

void DespillAPIop::knobs(Knob_Callback f)
{
  Enumeration_knob(f, &k_colorType, Constants::COLOR_TYPES, "color");
  ClearFlags(f, Knob::STARTLINE);
  Bool_knob(f, &k_absMode, "absoluteMode", "Absolute Mode");

  Knob *pick_knob = Color_knob(f, k_spillPick, "pick");
  ClearFlags(f, Knob::MAGNITUDE | Knob::SLIDER);

  Enumeration_knob(f, &k_despillMath, Constants::DESPILL_MATH_TYPES, "despillMath", "math");
  Float_knob(f, &k_customWeight, "customWeight", "");
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
  Knob *protectColor_knob = Color_knob(f, k_protectColor, "protectColor", "color");
  ClearFlags(f, Knob::MAGNITUDE | Knob::SLIDER);
  SetFlags(f, Knob::DISABLED);

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
  Knob *respillColor_knob = Color_knob(f, k_respillColor, "respillColor", "color");
  ClearFlags(f, Knob::MAGNITUDE | Knob::SLIDER);
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
    Knob *customWeight_knob = k->knob("customWeight");
    if(despillMath_knob->get_value() == 3) {
      customWeight_knob->enable();
    }
    else {
      customWeight_knob->disable();
    }
    return 1;
  }

  if(k->is("color")) {
    if(knob("color")->get_value() != Constants::COLOR_PICK) {
      knob("pick")->disable();
    }
    else {
      knob("pick")->enable();
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
  bool isConnected = (inputOp && inputOp->node_name() != std::string("Black in root"));

  switch(i) {
    case inputSource:
      isSourceConnected = isConnected;
      break;
    case inputLimit:
      isLimitConnected = isConnected;
      break;
    case inputColor:
      isColorConnected = isConnected;
      break;
    case inputRespill:
      isRespillConnected = isConnected;
      break;
  }

  if(isColorConnected) {
    knob("pick")->disable();
    knob("color")->disable();
  }
  else {
    knob("pick")->enable();
    knob("color")->enable();
  }
}

void DespillAPIop::_validate(bool for_real)
{
  copy_info(0);

  nuke::ChannelSet outChannels = channels();
  outChannels += k_outputSpillChannel;
  set_out_channels(outChannels);
  info_.turn_on(outChannels);

  normVec = Vector3(1.0f, 1.0f, 1.0f);

  Vector3 pickSpill(k_spillPick);

  if(isColorConnected) {
    _clr = 0;
    _usePickedColor = 1;
  }
  else if(k_colorType != Constants::COLOR_PICK) {
    _usePickedColor = 0;
    _clr = k_colorType;
  }
  else if(pickSpill.x == pickSpill.y && pickSpill.x == pickSpill.z) {
    _returnColor = 1;
  }
  else {
    _usePickedColor = 1;
    _clr = 0;
  }

  if(!isColorConnected) {
    float _autoShift = 0.0f;
    if(_usePickedColor == 1) {
      Vector3 v1 = color::VectorToPlane(k_spillPick, normVec);
      Vector3 v2 = color::VectorToPlane(Vector3(1.0f, 0.0f, 0.0f), normVec);
      _autoShift = color::ColorAngle(v1, v2);
      _autoShift = _autoShift * 180.0f / M_PI_F;
    }
    _hueShift = k_hueOffset - _autoShift;
  }
}

void DespillAPIop::_request(int x, int y, int r, int t, ChannelMask channels, int count)
{
  nuke::ChannelSet requestedChannels = channels;
  requestedChannels += Mask_RGB;

  input(inputSource)->request(input(inputSource)->info().box(), requestedChannels, Mask_RGB);

  if(input(inputLimit) != nullptr) {
    input(inputLimit)->request(Mask_All, count);
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

  // Get input color row
  Row color_row(x, r);
  if(input(inputColor) != nullptr) {
    color_row.get(*input(inputColor), y, x, r, Mask_RGB);
  }

  // Get input respill row
  Row respill_row(x, r);
  if(input(inputRespill) != nullptr) {
    respill_row.get(*input(inputRespill), y, x, r, Mask_RGB);
  }

  // Get input limit channel
  Row limit_matte_row(x, r);
  const float *limitPtr;
  if(input(inputLimit) != nullptr) {
    limit_matte_row.get(*input(inputLimit), y, x, r, Mask_All);
  }
  if(input(inputLimit) != nullptr) {
    limitPtr = limit_matte_row[k_limitChannel] + x;
  }

  Vector3 rgb;
  Vector3 colorRgb;
  Vector3 respillRgb;
  Vector3 spillLuma;
  imgcore::Pixel<const float> colorPixel(3);
  imgcore::Pixel<const float> respillPixel(3);
  imgcore::Pixel<const float> inPixel(3);
  imgcore::Pixel<float> outPixel(3);

  // increment helper
  auto incrementPointers = [&]() {
    inPixel++;
    outPixel++;
    if(input(inputColor) != nullptr) {
      colorPixel++;
    }
    if(input(inputRespill) != nullptr) {
      respillPixel++;
    }
    if(input(inputLimit) != nullptr) {
      limitPtr++;
    }
  };

  for(int i = 0; i < 3; ++i) {
    inPixel.SetPtr(row[static_cast<nuke::Channel>(i + 1)] + x, i);
    colorPixel.SetPtr(color_row[static_cast<nuke::Channel>(i + 1)] + x, i);
    respillPixel.SetPtr(respill_row[static_cast<nuke::Channel>(i + 1)] + x, i);
    outPixel.SetPtr(row.writable(static_cast<nuke::Channel>(i + 1)) + x, i);
  }

  // Loop through the pixels
  for(int x0 = x; x0 < r; ++x0) {
    for(int i = 0; i < 3; i++) {
      rgb[i] = inPixel.GetVal(i);
      colorRgb[i] = colorPixel.GetVal(i);
      respillRgb[i] = respillPixel.GetVal(i);
    }

    if(_returnColor == 1) {
      incrementPointers();
      continue;
    }

    float spillMatte = 0.0f;
    float hueShift = 0.0f;
    float autoShift = 0.0f;
    Vector3 despillColor;

    // Hue
    if(isColorConnected) {
      // Lee el color del input 'color'
      despillColor = colorRgb;
      // caclc red angle
      Vector3 v1 = color::VectorToPlane(k_spillPick);
      Vector3 v2 = color::VectorToPlane(Vector3(1.0f, 0.0f, 0.0f));
      autoShift = color::ColorAngle(v1, v2);
      autoShift = autoShift * 180.0f / M_PI_F;
      hueShift = k_hueOffset - autoShift;
    }
    else {
      if(_usePickedColor == 1) {
        despillColor = Vector3(k_spillPick);
      }
      else {
        despillColor =
            Vector3(_clr == 0 ? 1.0f : 0.0f, _clr == 1 ? 1.0f : 0.0f, _clr == 2 ? 1.0f : 0.0f);
      }
      hueShift = _hueShift;
    }

    // Limit
    float invertInputLimit = k_invertLimitMask ? (1.0f - (*limitPtr)) : *limitPtr;
    float limitResult = isLimitConnected ? k_hueLimit * invertInputLimit : k_hueLimit;

    // Despill
    Vector4 despilled = color::Despill(rgb, hueShift, _clr, k_despillMath, limitResult,
                                       k_customWeight, k_protectTones, k_protectColor,
                                       k_protectTolerance, k_protectEffect, k_protectFalloff);

    if(k_protectPrev && k_protectTones) {
      for(int i = 0; i < 3; i++) {
        outPixel[i] = rgb[i] * clamp(despilled[3] * k_protectEffect, 0.0f, 1.0f);
      }
      incrementPointers();
      continue;
    }

    // calcula el spill
    Vector4 spill = Vector4(rgb[0], rgb[1], rgb[2], 1.0f) - despilled;
    float spillLuma = color::GetLuma(spill, k_respillMath);

    // procesa el key
    Vector4 result;
    Vector4 despilledFull, spillFull;
    float spillLumaFull;

    if(!k_absMode) {
      despilledFull = despilled;
      spillFull = spill;
      spillLumaFull = spillLuma;
    }
    else {
      Vector4 despillColor4 = Vector4(despillColor.x, despillColor.y, despillColor.z, 1.0f);

      Vector4 pickSpillDespilled = color::Despill(
          despillColor, hueShift, _clr, k_despillMath, limitResult, k_customWeight, k_protectTones,
          k_protectColor, k_protectTolerance, k_protectEffect, k_protectFalloff);
      Vector4 pickSpillSp = despillColor4 - pickSpillDespilled;

      float pickSpillSpLuma = color::GetLuma(pickSpillSp, k_respillMath);

      // normaliza el spill
      spillLumaFull = pickSpillSpLuma == 0.0f ? 0.0f : spillLuma / pickSpillSpLuma;
      spillFull = despillColor4 * spillLumaFull;
      despilledFull = Vector4(rgb[0], rgb[1], rgb[2], 1.0f) - spillFull;
    }

    // calcula el color respill final
    Vector4 respillRgb4(respillRgb[0], respillRgb[1], respillRgb[2], 1.0f);
    Vector4 respillColor(k_respillColor[0], k_respillColor[1], k_respillColor[2], 1.0f);
    Vector4 respillColorResult = isRespillConnected ? respillRgb4 * respillColor : respillColor;

    if(k_outputType == Constants::OUTPUT_DESPILL) {
      result = despilledFull +
               Vector4(spillLumaFull, spillLumaFull, spillLumaFull, 1.0f) * respillColorResult;
    }
    else {
      result = spillFull;
    }

    // Write RGB channels to the output
    for(int i = 0; i < 3; i++) {
      outPixel[i] = result[i];
    }

    // Write spill channel
    foreach(z, channels) {
      if(z == k_outputSpillChannel) {
        *(row.writable(z) + x0) = 1.0f;  // output spill alpha
      }
    }

    incrementPointers();
  }
}

static Iop *build(Node *node)
{
  return (new NukeWrapper(new DespillAPIop(node)))->noChannels();
}
const Iop::Description DespillAPIop::d("DespillAP", "Keyer/DespillAP", build);