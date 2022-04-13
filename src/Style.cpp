//
// Created by alex2772 on 2/16/21.
//

#include <AUI/ASS/ASS.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/View/AText.h>
#include "Window/MainWindow.h"

using namespace ass;

struct Style {
    Style() {
        AStylesheet::inst().addRules({
            {
                t<MainWindow>(),
                Padding { 0 },
            },
            {
                c("#play"),
                Padding { 16_dp },
                FixedSize { 300_dp, 50_dp },
            },
            {
                c("#play") >> t<ADrawableView>(),
                FixedSize { 20_dp },
                Margin { 4_dp },
            },
            {
                c("#play") >> t<ALabel>(),
                FontSize { 16_pt },
                TextColor { 0xffffff_rgb },
                Margin { 0, 8_dp },
                TextAlign::CENTER,
            },
            {
                c("#downloading_panel"),
                FixedSize { 300_dp, 50_dp },
                Border { 1_px, 0x888888_rgb },
                BorderRadius { 4_dp },
                Expanding {},
                Margin {2_dp, 4_dp},
            },
            {
                c("#settings"),
                BackgroundImage { ":svg/cog.svg", 0xccffffff_argb, {}, Sizing::FIT_PADDING },
                FixedSize { 26_dp },
                Padding { 2_dp },
            },
            {
                c(".plus"),
                Padding { 4_dp },
            },
            {
                c(".configure"),
                BackgroundImage { ":svg/wrench.svg", 0x0_rgb, {}, Sizing::FIT_PADDING },
                MinSize { 14_dp },
                Padding { 4_dp },
                Margin { {}, 0, {} },
            },
            {
                c(".column"),
                Padding { {}, 4_dp },
                Expanding { {}, 1 },
            },
            {
                c(".title"),
                Padding { 4_dp, 0 },
                FontSize { 20_pt },
            },
            {
                c(".import_version_offset"),
                Margin { {}, {}, {}, 22_dp },
            },
            {
                c(".import_version_offset") >> t<AButton>(),
                Margin { 0 },
            },
            {
                c(".secondary"),
                TextColor { 0x444444_rgb },
            },
            {
                c(".version_item"),
                Padding { 8_dp },
                Margin { 4_dp },
            },
            {
                c::hover(".version_item"),
                BackgroundSolid { 0x10000000_argb },
            },
            {
                c(".version_item_selected"),
                BackgroundSolid { 0x22000000_argb },
                Border { 1_px, 0x40000000_argb },
            },
            {
                c(".version_item_wrap") > t<ADrawableView>(),
                MinSize { 48_dp },
            },
            {
                c(".version_item_wrap"),
                MinSize { 48_dp },
            },
            {
                c(".version_item") > t<AText>(),
                TextAlign::CENTER,
                Margin { 10_dp, 0, 0, 0 },
            },
            {
                c(".spinner_overlay"),
                BackgroundSolid { 0x80ffffff_argb },
                Expanding {},
            },
            {
                c(".console_wrap"),
                BackgroundSolid { 0x202020_rgb },
                Expanding {},
                Padding { 4_dp },
            },
            {
                c(".console_wrap") >> t<AView>(),
                TextColor { AColor::WHITE },
                Font { ":font/JetBrainsMono-Regular.ttf" },
                Margin { 0 },
                LineHeight { 1.5 },
            },
        });
    }
} s;