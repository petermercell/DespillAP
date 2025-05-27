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
  // copy image info
  copy_info(0);

  // setup output channels:
  // include all requested channels plus our spill output channel
  nuke::ChannelSet outChannels = channels();
  outChannels += k_outputSpillChannel;
  set_out_channels(outChannels);
  info_.turn_on(outChannels);

  // initialize normalization vector for colorspace calcs
  normVec = Vector3(1.0f, 1.0f, 1.0f);

  // get the picked spill color
  Vector3 pickSpill(k_spillPick);

  // determine color selection mode
  // and setup internal variables
  if(isColorConnected) {
    // color input is connected:
    // use automatic color detection
    _clr = 0;             // red channel
    _usePickedColor = 1;  // flag to use picked color
  }
  else if(k_colorType != Constants::COLOR_PICK) {
    // manual channel selection (Red/Green/Blue butons)
    _usePickedColor = 0;  // use channel selection, not picked color
    _clr = k_colorType;   // use selected channel (0=Red, 1=Green, 2=Blue)
  }
  else if(pickSpill.x == pickSpill.y && pickSpill.x == pickSpill.z) {
    // if picked color is grayscale (all rgb values equal)
    // this means that no valid color was picked,
    // so pass trought input unchanged
    _returnColor = 1;  // bypass all processing
  }
  else {
    // valid color was picked from picker knob
    _usePickedColor = 1;  // use the picked color
    _clr = 0;             // default processing channel
  }

  // calculate hue shift for non connected color mode
  if(!isColorConnected) {
    float _autoShift = 0.0f;

    if(_usePickedColor == 1) {
      // calculate automatic hue shift based on picked color
      // convert picked color and red reference to plane vectors for angle calc
      Vector3 v1 = color::VectorToPlane(k_spillPick, normVec);
      Vector3 v2 = color::VectorToPlane(Vector3(1.0f, 0.0f, 0.0f), normVec);  // red reference

      // calculate angle between picked color and red reference
      _autoShift = color::ColorAngle(v1, v2);
      _autoShift = _autoShift * 180.0f / M_PI_F;  // rads to deg
    }

    // final hue shift: user offset - automatic shift
    // this allows user to fine-tune the calculated shift
    _hueShift = k_hueOffset - _autoShift;
  }
}

void DespillAPIop::_request(int x, int y, int r, int t, ChannelMask channels, int count)
{
  // ensure RGB channels are always requested for processing
  nuke::ChannelSet requestedChannels = channels;
  requestedChannels += Mask_RGB;

  // request data fron input 'Source'
  input(inputSource)->request(input(inputSource)->info().box(), requestedChannels, Mask_RGB);

  // request limit matte if its connected to input 'Limit'
  if(input(inputLimit) != nullptr) {
    input(inputLimit)->request(Mask_All, count);
  };

  // request color reference if its connected to input 'Color'
  if(input(inputColor) != nullptr) {
    input(inputColor)->request(input(inputColor)->info().box(), Mask_RGB, count);
  };

  // request respill color if its connected to input 'Respill'
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
  // get main input data
  nuke::ChannelSet requestedChannels = channels;
  requestedChannels += Mask_RGBA;  // Add RGBA
  row.get(input0(), y, x, r, requestedChannels);

  // copy all non rgb channels
  nuke::ChannelSet copyMask = channels - nuke::Mask_RGB;
  row.pre_copy(row, copyMask);
  row.copy(row, copyMask, x, r);

  // get input color reference (for atm color detection)
  Row color_row(x, r);
  if(input(inputColor) != nullptr) {
    color_row.get(*input(inputColor), y, x, r, Mask_RGB);
  }

  // get optional respill color input (custom replacement color)
  Row respill_row(x, r);
  if(input(inputRespill) != nullptr) {
    respill_row.get(*input(inputRespill), y, x, r, Mask_RGB);
  }

  // get limit matte input
  Row limit_matte_row(x, r);
  const float *limitPtr;
  if(input(inputLimit) != nullptr) {
    limit_matte_row.get(*input(inputLimit), y, x, r, Mask_All);
  }
  if(input(inputLimit) != nullptr) {
    limitPtr = limit_matte_row[k_limitChannel] + x;
  }

  // get pointer to input alpha channel for pass-throught
  const float *input_alpha = row[Chan_Alpha] + x;

  Vector3 rgb;
  Vector3 colorRgb;
  Vector3 respillRgb;
  Vector3 spillLuma;

  // pixel pointers for efficient multichannel processing from afx tools
  imgcore::Pixel<const float> colorPixel(3);
  imgcore::Pixel<const float> respillPixel(3);
  imgcore::Pixel<const float> inPixel(3);
  imgcore::Pixel<float> outPixel(3);

  // lambda to increment all pixel pointers
  auto incrementPointers = [&]() {
    inPixel++;
    outPixel++;
    input_alpha++;
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

  // set pixel pointers to point to RGB channels
  for(int i = 0; i < 3; ++i) {
    inPixel.SetPtr(row[static_cast<nuke::Channel>(i + 1)] + x, i);
    colorPixel.SetPtr(color_row[static_cast<nuke::Channel>(i + 1)] + x, i);
    respillPixel.SetPtr(respill_row[static_cast<nuke::Channel>(i + 1)] + x, i);
    outPixel.SetPtr(row.writable(static_cast<nuke::Channel>(i + 1)) + x, i);
  }

  // Main pixel loop
  for(int x0 = x; x0 < r; ++x0) {
    // read rgb values from the current pixel
    for(int i = 0; i < 3; i++) {
      rgb[i] = inPixel.GetVal(i);
      colorRgb[i] = colorPixel.GetVal(i);
      respillRgb[i] = respillPixel.GetVal(i);
    }

    // early exit if color is unchanged
    if(_returnColor == 1) {
      incrementPointers();
      continue;
    }

    float spillMatte = 0.0f;
    float hueShift = 0.0f;
    float autoShift = 0.0f;
    Vector3 despillColor;

    // determine despill color and hue shift
    if(isColorConnected) {
      // use color from connected input for automatic color detection
      despillColor = colorRgb;
      Vector3 v1 = color::VectorToPlane(despillColor);
      Vector3 v2 = color::VectorToPlane(Vector3(1.0f, 0.0f, 0.0f));
      autoShift = color::ColorAngle(v1, v2);
      autoShift = autoShift * 180.0f / M_PI_F;  // rad to deg
      hueShift = k_hueOffset - autoShift;
    }
    else {
      // use manual color detection
      if(_usePickedColor == 1) {
        despillColor = Vector3(k_spillPick);
      }
      else {
        // create a color constant based on selected channel
        // 0=red, 1=green, 2=blue
        despillColor =
            Vector3(_clr == 0 ? 1.0f : 0.0f, _clr == 1 ? 1.0f : 0.0f, _clr == 2 ? 1.0f : 0.0f);
      }
      hueShift = _hueShift;
    }

    // apply limit matte if connected
    float invertInputLimit = k_invertLimitMask ? (1.0f - (*limitPtr)) : *limitPtr;
    float limitResult = isLimitConnected ? k_hueLimit * invertInputLimit : k_hueLimit;

    // perform limit operation
    Vector4 despilled = color::Despill(rgb, hueShift, _clr, k_despillMath, limitResult,
                                       k_customWeight, k_protectTones, k_protectColor,
                                       k_protectTolerance, k_protectEffect, k_protectFalloff);

    // case: if tones are protected, output protection matte
    if(k_protectPrev && k_protectTones) {
      for(int i = 0; i < 3; i++) {
        outPixel[i] = rgb[i] * clamp(despilled[3] * k_protectEffect, 0.0f, 1.0f);
      }
      // move to next pixel
      incrementPointers();
      continue;
    }

    // calculate spill amount (difference between original and despilled)
    Vector4 spill = Vector4(rgb[0], rgb[1], rgb[2], 1.0f) - despilled;
    float spillLuma = color::GetLuma(spill, k_respillMath);

    // process key generation and normalization
    Vector4 result;
    Vector4 despilledFull, spillFull;
    float spillLumaFull;

    if(!k_absMode) {
      // relative mode: use calculated values
      despilledFull = despilled;
      spillFull = spill;
      spillLumaFull = spillLuma;
    }
    else {
      // absolute mode: normalize spill relative to pícked color
      Vector4 despillColor4 = Vector4(despillColor.x, despillColor.y, despillColor.z, 0.0f);

      // calculate how much the pícked color would be despilled
      Vector4 pickSpillDespilled = color::Despill(
          despillColor, hueShift, _clr, k_despillMath, limitResult, k_customWeight, k_protectTones,
          k_protectColor, k_protectTolerance, k_protectEffect, k_protectFalloff);
      Vector4 pickSpillSp = despillColor4 - pickSpillDespilled;

      float pickSpillSpLuma = color::GetLuma(pickSpillSp, k_respillMath);

      // normalize current spill relative to picked color spill
      spillLumaFull = pickSpillSpLuma == 0.0f ? 0.0f : spillLuma / pickSpillSpLuma;
      spillFull = despillColor4 * spillLumaFull;
      despilledFull = Vector4(rgb[0], rgb[1], rgb[2], 0.0f) - spillFull;
      spillMatte = pickSpillDespilled[3];
    }

    // calculate final respill color (replacement color for removed spill)
    Vector4 respillRgb4(respillRgb[0], respillRgb[1], respillRgb[2], 0.0f);
    Vector4 respillColor(k_respillColor[0], k_respillColor[1], k_respillColor[2], 0.0f);
    Vector4 respillColorResult = isRespillConnected ? respillRgb4 * respillColor : respillColor;

    // output type: despilled image or spill matte
    if(k_outputType == Constants::OUTPUT_DESPILL) {
      // output despilled image with respill color added back
      result = despilledFull + Vector4(spillLumaFull, spillLumaFull, spillLumaFull, spillLumaFull) *
                                   respillColorResult;
    }
    else {
      result = spillFull;
    }

    // determine alpha output value
    if(!k_outputAlpha) {
      // pass the original input alpha channel
      spillMatte = *input_alpha;
    }
    else if(!k_invertAlpha) {
      // output spill amount as alpha channel
      spillMatte = spillLumaFull;
    }
    else {
      // output inverted spill amount as alpha channel
      spillMatte = 1 - spillLumaFull;
    }

    // write alpha channel to specified output channel
    foreach(z, channels) {
      if(z == k_outputSpillChannel) {
        *(row.writable(z) + x0) = clamp(spillMatte, 0.0f, 1.0f);  // output spill alpha
      }
    }

    // write RGB channels to output
    for(int i = 0; i < 3; i++) {
      outPixel[i] = result[i];
    }

    // move to next pixel
    incrementPointers();
  }
}

static Iop *build(Node *node)
{
  return (new NukeWrapper(new DespillAPIop(node)))->noChannels();
}
const Iop::Description DespillAPIop::d("DespillAP", "Keyer/DespillAP", build);