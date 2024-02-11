
#include "OssmUi.h"

// speed and encoder state

volatile int s_speed_percentage = 0;
volatile int s_encoder_position = 0;
String s_mode_label = "STROKE";
String s_message = "Machine Homing";

// Pentagram logo
const uint8_t km_logo[] PROGMEM = {
    0x00, 0xc0, 0xff, 0x03, 0x00, 0x00, 0xf0, 0xd3, 0x0f, 0x00, 0x00, 0x3c, 0x38, 0x3c, 0x00, 0x00, 0x0f, 0x38, 0xf0,
    0x00, 0x80, 0x03, 0x38, 0xc0, 0x01, 0xc0, 0x01, 0x7c, 0x80, 0x03, 0xe0, 0x00, 0x6c, 0x00, 0x07, 0x70, 0x00, 0x6c,
    0x00, 0x0e, 0x38, 0x00, 0xee, 0x00, 0x1c, 0x18, 0x00, 0xc6, 0x00, 0x18, 0x0c, 0x00, 0xc6, 0x00, 0x30, 0x0c, 0x00,
    0xc6, 0x00, 0x30, 0x06, 0x00, 0x83, 0x01, 0x60, 0xfe, 0xff, 0xff, 0xff, 0x7f, 0xf3, 0xff, 0xff, 0xff, 0xdf, 0xe3,
    0x81, 0x83, 0x03, 0xcf, 0xc3, 0x83, 0x01, 0x83, 0xc7, 0x03, 0x87, 0x01, 0xc3, 0xc1, 0x01, 0xde, 0x01, 0xf7, 0x80,
    0x01, 0xfc, 0x00, 0x7e, 0x80, 0x01, 0xf0, 0x00, 0x1e, 0x80, 0x01, 0xe0, 0x01, 0x0f, 0x80, 0x03, 0xe0, 0x83, 0x0f,
    0xc0, 0x03, 0x60, 0xc7, 0x0d, 0xc0, 0x03, 0x70, 0xfe, 0x1c, 0xc0, 0x03, 0x30, 0x7c, 0x18, 0xc0, 0x06, 0x30, 0x7c,
    0x18, 0x60, 0x06, 0x38, 0xef, 0x19, 0x60, 0x0c, 0x98, 0x83, 0x33, 0x30, 0x0c, 0xd8, 0x01, 0x37, 0x30, 0x18, 0xf8,
    0x00, 0x3e, 0x18, 0x38, 0x3c, 0x00, 0x78, 0x1c, 0x70, 0x1c, 0x00, 0x70, 0x0e, 0xe0, 0x0c, 0x00, 0x60, 0x07, 0xc0,
    0x03, 0x00, 0x80, 0x03, 0x80, 0x03, 0x00, 0xc0, 0x01, 0x00, 0x0f, 0x00, 0xf0, 0x00, 0x00, 0x3c, 0x00, 0x3c, 0x00,
    0x00, 0xf0, 0xc3, 0x0f, 0x00, 0x00, 0xc0, 0xff, 0x03, 0x00};

// default activity symbols

const uint8_t s_active_symbol[8] PROGMEM = {B00000000, B00000000, B00011000, B00100100,
                                            B01000010, B01000010, B00100100, B00011000};

const uint8_t s_inactive_symbol[8] PROGMEM = {B00000000, B00000000, B00000000, B00000000,
                                              B00011000, B00011000, B00000000, B00000000};

// default overlays and frames

static void OssmUiOverlaySpeed(OLEDDisplay* display, OLEDDisplayUiState* state)
{
    display->setFont(ArialMT_Plain_10);

    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, 0, "SPEED");

    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    // int offset = display->getWidth() - display->getStringWidth(s_mode_label);
    display->drawString(display->getWidth(), 0, s_mode_label);

    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(64, 50, s_message);
}
static void OssmUiOverlayBooting(OLEDDisplay* display, OLEDDisplayUiState* state)
{
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(ArialMT_Plain_10);
    display->drawString(64, 0, " TEST                          ");
    display->drawString(64, 50, s_message);
}
static void OssmUiFrameKMlogo(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    // display->fillRect(10,14+(50-int(s_speed_percentage/2)),10,20-int(s_speed_percentage/100));
    display->fillRect(10, 14 + (34 - int(s_speed_percentage / 3)), 10, int(s_speed_percentage / 3));
    display->drawXbm(x + 44, y + 6, 40, 40, km_logo);
    // display->fillRect(106,64-int(s_encoder_position/2),10,64);
    display->fillRect(106, 14 + (34 - int(s_encoder_position / 3)), 10, int(s_encoder_position / 3));
}

const size_t s_overlay_count = 1;
OverlayCallback s_overlays[] = {OssmUiOverlaySpeed};

const size_t s_frame_count = 1;
FrameCallback s_frames[] = {OssmUiFrameKMlogo};

// OssmUi constructor and methods

OssmUi::OssmUi(uint8_t address, int sda, int scl)
    : m_display(address, sda, scl),
      m_ui(&m_display),
      m_address(address),
      m_check_connectivity_interval(250),
      m_connected(false),
      m_last_update_time(0)
{
    // fps
    m_ui.setTargetFPS(20);
    // activity indicators
    m_ui.setActiveSymbol(s_active_symbol);
    m_ui.setInactiveSymbol(s_inactive_symbol);
    // frames
    m_ui.setFrames(s_frames, s_frame_count);
    // overlays
    m_ui.setOverlays(s_overlays, s_overlay_count);
    // no auto transition
    m_ui.disableAutoTransition();
    // no transition animation
    m_ui.setTimePerTransition(0);
    // Get rid of the indicators on screen
    m_ui.disableAllIndicators();
}

void OssmUi::showBootScreen()
{
    // m_ui.switchToFrame(1);
    //  const size_t s_overlay_count = 1;
    //  OverlayCallback s_overlays[] = {OssmUiOverlayBooting};
    //  const size_t s_frame_count = 1;
    //  FrameCallback s_frames[] = {OssmUiFrameKMlogo};
    //  m_ui.setFrames(s_frames, s_frame_count);
    //  m_ui.setOverlays(s_overlays, s_overlay_count);
}

void OssmUi::SetTargetFps(uint8_t target_fps)
{
    m_ui.setTargetFPS(target_fps);
}

void OssmUi::ResetState()
{
    m_display.end();
    m_display.init();
    m_display.flipScreenVertically();
    // m_ui.switchToFrame(0);
}

void OssmUi::SetFrames(OverlayCallback* overlays, size_t overlays_count, FrameCallback* frames, size_t frames_count)
{
    m_ui.setFrames(frames, frames_count);
    m_ui.setOverlays(overlays, overlays_count);
}

void OssmUi::SetActivitySymbols(const uint8_t* active, const uint8_t* inactive)
{
    m_ui.setActiveSymbol(active);
    m_ui.setInactiveSymbol(inactive);
}

void OssmUi::Setup()
{
    // OLED SETUP

    // You can change this to
    // TOP, LEFT, BOTTOM, RIGHT
    m_ui.setIndicatorPosition(LEFT);

    // Defines where the first frame is located in the bar.
    m_ui.setIndicatorDirection(LEFT_RIGHT);

    // Initialising the UI will init the display too.
    m_ui.init();

    // flip screen
    m_display.flipScreenVertically();
}

void OssmUi::UpdateState(String mode_label, const int speed_percentage, const int encoder_position)
{
    s_mode_label = mode_label;
    s_speed_percentage = speed_percentage;
    s_encoder_position = encoder_position;
}

void OssmUi::UpdateMessage(String message_in)
{
    s_message = message_in;
    m_ui.update();
}

void OssmUi::UpdateScreen()
{
    m_ui.update();

    // periodically check connectivity
    uint32_t now = millis();
    if (now - m_last_update_time > m_check_connectivity_interval)
    {
        m_last_update_time = now;
        Wire.beginTransmission(m_address);
        if (0 == Wire.endTransmission(m_address))
        {
            if (!m_connected)
            {
                ResetState();
            }
            m_connected = true;
        }
        else
        {
            m_connected = false;
        }
    }
}

void OssmUi::UpdateOnly()
{
    m_ui.update();
}
