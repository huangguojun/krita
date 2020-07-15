#include "kis_my_paintop_settings_widget.h"

#include "kis_my_paintop_settings.h"

#include <kis_color_option.h>
#include <kis_paintop_settings_widget.h>
#include <kis_paint_action_type_option.h>

#include <kis_brush_option_widget.h>
#include <kis_compositeop_option.h>
#include <kis_my_paintop_option.h>
#include <kis_mypaint_curve_option.h>
#include <kis_mypaint_curve_option_widget.h>

KisMyPaintOpSettingsWidget:: KisMyPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    addPaintOpOption(new KisMyPaintOpOption(), i18n("Basic"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("radius_logarithmic", KisPaintOpOption::GENERAL, false), "0", "100"), "Radius Logarithmic", "Basic");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("radius_by_random", KisPaintOpOption::GENERAL, false), "0", "100"), "Radius by Random", "Basic");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("hardness", KisPaintOpOption::GENERAL, false), "0", "100"), "Hardness", "Basic");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("anti_aliasing", KisPaintOpOption::GENERAL, false), "0", "100"), "Anti Aliasing", "Basic");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("elliptical_dab_angle", KisPaintOpOption::GENERAL, false), "0", "100"), "Elliptical Dab Angle", "Basic");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("elliptical_dab_ratio", KisPaintOpOption::GENERAL, false), "0", "100"), "Elliptical Dab Ratio", "Basic");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("direction_filter", KisPaintOpOption::GENERAL, false), "0", "100"), "Direction Filter", "Basic");
//    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("color_h", KisPaintOpOption::GENERAL, false), "0", "100"), "Color H", "Color");
//    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("color_s", KisPaintOpOption::GENERAL, false), "0", "100"), "Color S", "Color");
//    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("color_v", KisPaintOpOption::GENERAL, false), "0", "100"), "Color V", "Color");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("change_color_h", KisPaintOpOption::GENERAL, false), "0", "100"), "Change Color H", "Color");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("change_color_l", KisPaintOpOption::GENERAL, false), "0", "100"), "Change Color L", "Color");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("change_color_v", KisPaintOpOption::GENERAL, false), "0", "100"), "Change Color V", "Color");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("change_color_hsl_s", KisPaintOpOption::GENERAL, false), "0", "100"), "Change Color HSL S", "Color");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("change_color_hsv_s", KisPaintOpOption::GENERAL, false), "0", "100"), "Change Color HSV S", "Color");            
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("colorize", KisPaintOpOption::GENERAL, false), "0", "100"), "Colorize", "Color");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("speed1_gamma", KisPaintOpOption::GENERAL, false), "0", "100"), "Fine Speed Gamma", "Speed");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("speed2_gamma", KisPaintOpOption::GENERAL, false), "0", "100"), "Gross Speed Gamma", "Speed");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("speed1_slowness", KisPaintOpOption::GENERAL, false), "0", "100"), "Fine Speed Slowness", "Speed");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("speed2_slowness", KisPaintOpOption::GENERAL, false), "0", "100"), "Gross Speed Slowness", "Speed");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("offset_by_random", KisPaintOpOption::GENERAL, false), "0", "100"), "Offset By Random", "Speed");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("offset_by_speed", KisPaintOpOption::GENERAL, false), "0", "100"), "Offset By Speed", "Speed");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("offset_by_speed_slowness", KisPaintOpOption::GENERAL, false), "0", "100"), "Offset By Speed Slowness", "Speed");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("dabs_per_actual_radius", KisPaintOpOption::GENERAL, false), "0", "100"), "Dabs Per Actual Radius", "Dabs");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("dabs_per_second", KisPaintOpOption::GENERAL, false), "0", "100"), "Dabs per Second", "Dabs");   
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("opaque", KisPaintOpOption::GENERAL, false), "0", "100"), "Opaque", "Opacity");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("opaque_linearize", KisPaintOpOption::GENERAL, false), "0", "100"), "Opaque Linearize", "Opacity");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("opaque_multiply", KisPaintOpOption::GENERAL, false), "0", "100"), "Opaque Multiply", "Opacity");    
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("slow_tracking_per_dab", KisPaintOpOption::GENERAL, false), "0", "100"), "Slow tracking per dab", "Tracking");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("slow_tracking", KisPaintOpOption::GENERAL, false), "0", "100"), "Slow Tracking", "Tracking");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("tracking_noise", KisPaintOpOption::GENERAL, false), "0", "100"), "Tracking Noise", "Tracking");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("smudge", KisPaintOpOption::GENERAL, false), "0", "100"), "Smudge", "Smudge");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("smudge_length", KisPaintOpOption::GENERAL, false), "0", "100"), "Smudge Length", "Smudge");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("smudge_radius_log", KisPaintOpOption::GENERAL, false), "0", "100"), "Smudge Radius Log", "Smudge");   
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("stroke_duration_logarithmic", KisPaintOpOption::GENERAL, false), "0", "100"), "Stroke Duration log", "Stroke");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("stroke_holdtime", KisPaintOpOption::GENERAL, false), "0", "100"), "Stroke Holdtime", "Stroke");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("stroke_threshold", KisPaintOpOption::GENERAL, false), "0", "100"), "Stroke Threshold", "Stroke");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("custom_input", KisPaintOpOption::GENERAL, false), "0", "100"), "Custom Input", "Custom");
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("custom_input_slowness", KisPaintOpOption::GENERAL, false), "0", "100"), "Custom Input Slowness", "Custom");
}

KisMyPaintOpSettingsWidget::~ KisMyPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisMyPaintOpSettingsWidget::configuration() const
{
    KisMyPaintOpSettings* config = new KisMyPaintOpSettings();
    config->setOptionsWidget(const_cast<KisMyPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "mypaintbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}