//
// Created by alex2772 on 2/16/21.
//

#include <AUI/ASS/ASS.h>

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
        });
    }
} s;