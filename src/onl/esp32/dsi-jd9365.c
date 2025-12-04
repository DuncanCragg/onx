
#include <sdkconfig.h>
#include <soc/soc_caps.h>
#include <rom/ets_sys.h>

#include <esp_private/startup_internal.h>
#include <esp_private/periph_ctrl.h>
#include <esp_private/esp_clk_tree_common.h>

#include <esp_bit_defs.h>
#include <esp_check.h>
#include <esp_psram.h>
#include <esp_heap_caps_init.h>
#include <esp_heap_caps.h>
#include <esp_memory_utils.h>
#include <esp_ldo_regulator.h>
#include <esp_clk_tree.h>
#include <esp_pm.h>
#include <esp_log.h>

#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_io_interface.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_mipi_dsi.h>
#include <esp_lcd_jd9365_10_1.h>

#include <onx/dsi.h>

#define TEST_LCD_BIT_PER_PIXEL 24
#define TEST_PIN_NUM_LCD_RST -1
#define TEST_MIPI_DSI_LANE_NUM 2

#if TEST_LCD_BIT_PER_PIXEL == 24
#define TEST_MIPI_DPI_PX_FORMAT (LCD_COLOR_PIXEL_FORMAT_RGB888)
#elif TEST_LCD_BIT_PER_PIXEL == 18
#define TEST_MIPI_DPI_PX_FORMAT (LCD_COLOR_PIXEL_FORMAT_RGB666)
#elif TEST_LCD_BIT_PER_PIXEL == 16
#define TEST_MIPI_DPI_PX_FORMAT (LCD_COLOR_PIXEL_FORMAT_RGB565)
#endif

#define TEST_MIPI_DSI_PHY_PWR_LDO_CHAN (3)
#define TEST_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV (2500)

static esp_ldo_channel_handle_t  ldo_mipi_phy = 0;
static esp_lcd_panel_handle_t    panel = 0;
static esp_lcd_dsi_bus_handle_t  mipi_dsi_bus = 0;
static esp_lcd_panel_io_handle_t mipi_dbi_io = 0;

IRAM_ATTR static bool test_notify_refresh_ready(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx) {
    return 0;
}

void* dsi_init(){

  esp_ldo_channel_config_t ldo_mipi_phy_config = {
      .chan_id = TEST_MIPI_DSI_PHY_PWR_LDO_CHAN,
      .voltage_mv = TEST_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
  };
  esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy);

  esp_lcd_dsi_bus_config_t bus_config = JD9365_PANEL_BUS_DSI_2CH_CONFIG();
  esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus);

  esp_lcd_dbi_io_config_t dbi_config = JD9365_PANEL_IO_DBI_CONFIG();
  esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io);

  esp_lcd_dpi_panel_config_t dpi_config = JD9365_800_1280_PANEL_60HZ_DPI_CONFIG(TEST_MIPI_DPI_PX_FORMAT);
  jd9365_vendor_config_t vendor_config = {
      .flags = {
          .use_mipi_interface = 1,
      },
      .mipi_config = {
          .dsi_bus = mipi_dsi_bus,
          .dpi_config = &dpi_config,
          .lane_num = TEST_MIPI_DSI_LANE_NUM,
      },
  };
  const esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = TEST_PIN_NUM_LCD_RST,
      .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
      .bits_per_pixel = TEST_LCD_BIT_PER_PIXEL,
      .vendor_config = &vendor_config,
  };

  esp_lcd_new_panel_jd9365(mipi_dbi_io, &panel_config, &panel);

  if(!panel){ printf("can't create panel\n"); return 0; }

  esp_lcd_panel_reset(panel);

  esp_lcd_panel_init(panel);

  esp_lcd_panel_disp_on_off(panel, true);

  esp_lcd_dpi_panel_event_callbacks_t cbs = {
      .on_color_trans_done = test_notify_refresh_ready,
  };
  esp_lcd_dpi_panel_register_event_callbacks(panel, &cbs, 0);

  return panel;
}

void dsi_draw_bitmap(void* panel, int x_start, int y_start, int x_end, int y_end, const void* buf){
  esp_lcd_panel_draw_bitmap((esp_lcd_panel_handle_t)panel, x_start, y_start, x_end, y_end, buf);
}



