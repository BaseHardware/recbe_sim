#include <algorithm>
#include <vector>

#include "TColor.h"
#include "TStyle.h"
#include "THStack.h"

void SetMatplotlibTab10Palette(Int_t nColors = 10)
{
    // Matplotlib default color cycle, i.e. tab10 / C0...C9
    static const char* mplHex[10] = {
        "#1f77b4", // C0 tab:blue
        "#ff7f0e", // C1 tab:orange
        "#2ca02c", // C2 tab:green
        "#d62728", // C3 tab:red
        "#9467bd", // C4 tab:purple
        "#8c564b", // C5 tab:brown
        "#e377c2", // C6 tab:pink
        "#7f7f7f", // C7 tab:gray
        "#bcbd22", // C8 tab:olive
        "#17becf"  // C9 tab:cyan
    };

    static Int_t mplColorId[10];
    static bool initialized = false;

    if (!initialized) {
        for (Int_t i = 0; i < 10; ++i) {
            // ROOT color index from "#rrggbb"
            mplColorId[i] = TColor::GetColor(mplHex[i]);
        }
        initialized = true;
    }

    // 중요:
    // THStack PLC가 팔레트를 히스토그램 개수에 맞춰 샘플링하므로,
    // nColors를 히스토그램 개수로 맞추면 C0, C1, C2, ... 순서가 유지된다.
    const Int_t n = std::max<Int_t>(1, nColors);

    static std::vector<Int_t> palette;
    palette.resize(n);

    for (Int_t i = 0; i < n; ++i) {
        palette[i] = mplColorId[i % 10];
    }

    gStyle->SetPalette(n, palette.data());
}

void SetMatplotlibTab10Palette(THStack* hs)
{
    const Int_t n = hs ? std::max<Int_t>(1, hs->GetNhists()) : 10;
    SetMatplotlibTab10Palette(n);
}
