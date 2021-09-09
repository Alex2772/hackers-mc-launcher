//
// Created by alex2772 on 2/16/21.
//

#include <AUI/ASS/ASS.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AImageView.h>

using namespace ass;

struct Style {
    Style() {
        AStylesheet::inst().addRules({
            {
                c("#play"),
                BackgroundImage { ":svg/play.svg", 0xccffffff_argb, {}, Sizing::FIT_PADDING },
                Padding { 16_dp },
                FixedSize { 80_dp },
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
                Margin { {}, {}, {}, 30_dp },
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
                Margin { 0 },
            },
            {
                c::hover(".version_item"),
                BackgroundSolid { 0x10000000_argb },
            },
            {
                c(".version_item")["selected"],
                BackgroundSolid { 0x22000000_argb },
                Border { 1_px, 0x40000000_argb },
            },
            {
                c(".version_item_wrap") > t<AImageView>(),
                MinSize { 48_dp },
            },
            {
                c(".version_item_wrap"),
                MinSize { 48_dp },
            },
            {
                c(".version_item") > t<ALabel>(),
                TextAlign::CENTER,
                Margin { 10_dp, 0, 0, 0 },
            },
        });
    }
} s;