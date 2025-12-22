
#include <sdkconfig.h>
#include <soc/soc_caps.h>

#include <esp_private/startup_internal.h>
#include <esp_private/periph_ctrl.h>
#include <esp_private/esp_clk_tree_common.h>

#include <i2c_bus.h>

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
#include <esp_lcd_touch_gt911.h>

#include <onx/log.h>
#include <onx/dsi.h>

#define LCD_BIT_PER_PIXEL 24
#define PIN_NUM_LCD_RST -1
#define MIPI_DSI_LANE_NUM 2

#if LCD_BIT_PER_PIXEL == 24
#define MIPI_DPI_PX_FORMAT LCD_COLOR_PIXEL_FORMAT_RGB888
#elif LCD_BIT_PER_PIXEL == 18
#define MIPI_DPI_PX_FORMAT LCD_COLOR_PIXEL_FORMAT_RGB666
#elif LCD_BIT_PER_PIXEL == 16
#define MIPI_DPI_PX_FORMAT LCD_COLOR_PIXEL_FORMAT_RGB565
#endif

#define MIPI_DSI_PHY_PWR_LDO_CHAN 3
#define MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500

static esp_lcd_panel_handle_t panel = 0;
static esp_lcd_touch_handle_t touch = 0;

static volatile bool dma_done = false;

IRAM_ATTR static bool dma_done_cb(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx){
  dma_done = true;
  return false; // need yield
}

const uint16_t screen_width=1280;
const uint16_t screen_height=800;

void dsi_init(){

  esp_ldo_channel_config_t ldo_mipi_phy_config = {
    .chan_id = MIPI_DSI_PHY_PWR_LDO_CHAN,
    .voltage_mv = MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
  };
  esp_ldo_channel_handle_t ldo_mipi_phy = 0;
  esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy);

  esp_lcd_dsi_bus_config_t dsi_bus_config = JD9365_PANEL_BUS_DSI_2CH_CONFIG();
  esp_lcd_dsi_bus_handle_t mipi_dsi_bus = 0;
  esp_lcd_new_dsi_bus(&dsi_bus_config, &mipi_dsi_bus);

  esp_lcd_dbi_io_config_t dbi_config = JD9365_PANEL_IO_DBI_CONFIG();
  esp_lcd_panel_io_handle_t mipi_dbi_io = 0;
  esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io);

  esp_lcd_dpi_panel_config_t dpi_config = JD9365_800_1280_PANEL_60HZ_DPI_CONFIG(MIPI_DPI_PX_FORMAT);

  jd9365_vendor_config_t vendor_config = {
    .flags = {
      .use_mipi_interface = 1,
    },
    .mipi_config = {
      .dsi_bus = mipi_dsi_bus,
      .dpi_config = &dpi_config,
      .lane_num = MIPI_DSI_LANE_NUM,
    },
  };
  const esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = PIN_NUM_LCD_RST,
    .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
    .bits_per_pixel = LCD_BIT_PER_PIXEL,
    .vendor_config = &vendor_config,
  };

  esp_lcd_new_panel_jd9365(mipi_dbi_io, &panel_config, &panel);

; if(!panel){ log_write("can't create panel\n"); return; }

  esp_lcd_dpi_panel_event_callbacks_t cbs = {
      .on_color_trans_done = dma_done_cb,
  };
  esp_lcd_dpi_panel_register_event_callbacks(panel, &cbs, 0);

  esp_lcd_panel_reset(panel);
  esp_lcd_panel_init(panel);
  esp_lcd_panel_disp_on_off(panel, true);

  // -----------------------------------

  #define I2C_MASTER_NUM 0
  #define TOUCH_SDA_PIN  7
  #define TOUCH_SCL_PIN  8

  i2c_master_bus_config_t i2c_mst_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port   = I2C_MASTER_NUM,
      .sda_io_num = TOUCH_SDA_PIN,
      .scl_io_num = TOUCH_SCL_PIN,
      .flags.enable_internal_pullup = true,
  };
  i2c_master_bus_handle_t bus_handle;
  i2c_new_master_bus(&i2c_mst_config, &bus_handle);

  esp_lcd_panel_io_i2c_config_t i2c_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
  i2c_config.scl_speed_hz = 400*1000;
  esp_lcd_panel_io_handle_t touch_i2c = 0;
  esp_lcd_new_panel_io_i2c(bus_handle, &i2c_config, &touch_i2c);

  esp_lcd_touch_io_gt911_config_t touch_gt911_config = {
      .dev_addr = i2c_config.dev_addr,
  };

  esp_lcd_touch_config_t touch_config = {
      .x_max = screen_width,
      .y_max = screen_height,
      .rst_gpio_num = -1,
      .int_gpio_num = -1,
      .levels = {
        .reset     = 0,
        .interrupt = 0,
      },
      .flags = {
        .swap_xy = 0,
        .mirror_x = 0,
        .mirror_y = 0,
      },
      .driver_data = &touch_gt911_config,
  };

  esp_lcd_touch_new_i2c_gt911(touch_i2c, &touch_config, &touch);
}

extern void touch_i2c_event(uint16_t x[], uint16_t y[], uint8_t n);

void dsi_loop(){

  uint16_t x[5];
  uint16_t y[5];
  uint16_t s[5];
  uint8_t  n = 0;

  touch->read_data(touch);
  bool touched = touch->get_xy(touch, x,y,s, &n, 5);

  static bool pending_untouch=false;
  if(touched){
    touch_i2c_event(x,y,n);
    pending_untouch=true;
  }
  else
  if(pending_untouch){
    touch_i2c_event(0,0,0);
    pending_untouch=false;
  }
}

void dsi_draw_bitmap(void* buf, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t wait_for){
  if(!panel) return;
  dma_done=false;
  esp_lcd_panel_draw_bitmap(panel, x, y, x+w, y+h, buf);
  if(wait_for) for(uint32_t t=0; t< wait_for && !dma_done; t++) time_delay_us(1);
  dma_done=false;
}




