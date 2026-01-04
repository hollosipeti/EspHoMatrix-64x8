#include "esphome.h"
#ifdef EHMTXv2_MULTICOLOR_TEXT
#include <regex>
#endif

namespace esphome
{

  EHMTX_queue::EHMTX_queue(EHMTX *config)
  {
    this->config_ = config;
    this->endtime = 0.0;
    this->last_time = 0.0;
    this->screen_time_ = 0.0;
    this->mode = MODE_EMPTY;
    this->icon_name = "";
    this->icon = 0;
    this->text = "";
    this->default_font = true;
    this->progress = 0;
    this->sbitmap = nullptr;
    this->progressbar_color = esphome::display::COLOR_OFF;
    this->progressbar_back_color = esphome::display::COLOR_OFF;
  }

  void EHMTX_queue::status()
  {
    switch (this->mode)
    {
    case MODE_EMPTY:
      ESP_LOGD(TAG, "empty slot");
      break;
    case MODE_BLANK:
      ESP_LOGD(TAG, "queue: blank screen for %.1f sec", this->screen_time_ / 1000.0);
      break;
    case MODE_COLOR:
      ESP_LOGD(TAG, "queue: color screen for %.1f sec", this->screen_time_ / 1000.0);
      break;
    case MODE_CLOCK:
      ESP_LOGD(TAG, "queue: clock for: %.1f sec", this->screen_time_ / 1000.0);
      break;
    case MODE_DATE:
      ESP_LOGD(TAG, "queue: date for: %.1f sec", this->screen_time_ / 1000.0);
      break;
     case MODE_FULL_SCREEN:
      ESP_LOGD(TAG, "queue: full screen: \"%s\" for: %.1f sec", this->icon_name.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_ICON_SCREEN:
      ESP_LOGD(TAG, "queue: icon screen: \"%s\" text: %s for: %.1f sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_ICON_PROGRESS:
      ESP_LOGD(TAG, "queue: icon progress: \"%s\" text: %s for: %.1f sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_PROGNOSIS_SCREEN:
      ESP_LOGD(TAG, "queue: icon prognosis screen: \"%s\" text: %s for: %.1f sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_TEXT_PROGRESS:
      ESP_LOGD(TAG, "queue: text progress: \"%s\" text: %s for: %.1f sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_ICON_CLOCK:
      ESP_LOGD(TAG, "queue: icon clock: \"%s\" for: %.1f sec", this->icon_name.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_ICON_DATE:
      ESP_LOGD(TAG, "queue: icon date: \"%s\" for: %.1f sec", this->icon_name.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_ICON_DATE_TIME:
      ESP_LOGD(TAG, "queue: icon date time screen: \"%s\" for: %.1f sec", this->icon_name.c_str(), this->screen_time_ / 1000.0);
      break;  
      case MODE_ALERT_SCREEN:
      ESP_LOGD(TAG, "queue: alert icon: \"%s\" for: %.1f sec", this->icon_name.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_RAINBOW_ALERT_SCREEN:
      ESP_LOGD(TAG, "queue: rainbow alert icon: \"%s\" for: %.1f sec", this->icon_name.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_TEXT_SCREEN:
      ESP_LOGD(TAG, "queue: text text: \"%s\" for: %.1f sec", this->text.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_RAINBOW_ICON:
      ESP_LOGD(TAG, "queue: rainbow icon: \"%s\" text: %s for: %.1f sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_RAINBOW_TEXT:
      ESP_LOGD(TAG, "queue: rainbow text: \"%s\" for: %.1f sec", this->text.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_ICON_TEXT_SCREEN:
      ESP_LOGD(TAG, "queue: icon text screen: \"%s\" text: %s for: %.1f sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_ / 1000.0);
      break;
    case MODE_RAINBOW_ICON_TEXT_SCREEN:
      ESP_LOGD(TAG, "queue: rainbow icon text screen: \"%s\" text: %s for: %.1f sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_ / 1000.0);
      break;

    default:
      // Queue not initialized
      break;
    }
  }

  int32_t EHMTX_queue::xpos()
  {
    uint8_t width = 64;
    uint8_t startx = 0;
    int32_t result = 0;
    switch (this->mode)
    {
    case MODE_RAINBOW_ICON:
    case MODE_ICON_SCREEN:
    case MODE_ICON_CLOCK:
    case MODE_ICON_DATE:
    case MODE_ICON_DATE_TIME:
    case MODE_ALERT_SCREEN:
    case MODE_RAINBOW_ALERT_SCREEN:
    case MODE_ICON_PROGRESS:
    case MODE_PROGNOSIS_SCREEN:
      startx = 8;
      break;
    case MODE_TEXT_SCREEN:
    case MODE_RAINBOW_TEXT:
      // no correction
      break;
    case MODE_ICON_TEXT_SCREEN:
    case MODE_RAINBOW_ICON_TEXT_SCREEN:
      if (this->pixels_ < 64)
      {
        startx = 8;
      }
      break;
    default:
      break;
    }

    width -= startx;

#ifdef EHMTXv2_USE_RTL
    if (this->pixels_ < width)
    {
      result = 64 - ceil((width - this->pixels_) / 2);
    }
    else
    {

      result = startx + this->config_->scroll_step;
    }
#else
#ifdef EHMTXv2_SCROLL_SMALL_TEXT
    result = startx - this->config_->scroll_step + width;
#else
if (this->pixels_ < width)
    {
      // MÓDOSÍTÁS: Ha ikonos szöveg van, akkor BALRA zárjuk (mint az órát), ne középre
      if (this->mode == MODE_ICON_TEXT_SCREEN || this->mode == MODE_RAINBOW_ICON_TEXT_SCREEN) 
      {
        result = startx; 
      } 
      else 
      {
        // Minden más esetben (pl. sima szöveg ikon nélkül) marad a középre igazítás
        result = startx + ceil((width - this->pixels_) / 2);
      }
    }
    else
    {
      result = startx - this->config_->scroll_step + width;
    }
#endif
#endif
    return result;
  }

  void c16to8(int16_t t, uint8_t &r, uint8_t &g)
  {
    r = static_cast<uint8_t>((t & 0xFF00) >> 8);
    g = static_cast<uint8_t>(t & 0x00FF);
  }

  int16_t c8to16(uint8_t r, uint8_t g)
  {
    return (static_cast<uint16_t>(r) << 8) | g;
  }

  uint8_t is_tick(int32_t step, uint8_t &state)
  {
    if (step % 2 == state)
    {
      return 0;
    }
    state = step % 2;
    return 1;
  }

  int32_t EHMTX_queue::xpos(uint8_t item)
  {
    uint8_t width = 64;
    int32_t result = width - this->config_->scroll_step + item * 9;

    if (this->icon < 5)
    {
      int16_t item_pos = c8to16(this->sbitmap[item].r, this->sbitmap[item].g);

      uint8_t target = round(((static_cast<float>(width) - 8 * static_cast<float>(this->icon)) / static_cast<float>(this->icon + 1)) * (item + 1) + 8 * item);
      if ((this->progress == 1) && (this->icon == 2 || this->icon == 3))
      {
        uint8_t reverse_steps = round(((static_cast<float>(width) - 8 * static_cast<float>(this->icon)) / static_cast<float>(this->icon + 1)) + 8);

        if (ceil((this->config_->next_action_time - this->config_->get_tick()) / EHMTXv2_SCROLL_INTERVAL) > reverse_steps)
        {
          result = (item + 1 == this->icon) ? width - this->config_->scroll_step : -8 + this->config_->scroll_step;
          if (item == 0 && (item_pos == 32767 || item_pos < target))
          {
            item_pos = result < target ? result : target;
          }
          else if (item + 1 == this->icon && item_pos > target)
          {
            item_pos = result > target ? result : target;
          }
          else if (this->icon == 3 && item == 1)
          {
            item_pos = target;
          }
        }
        else
        {
          if (item == 0)
          {
            item_pos -= is_tick(this->config_->scroll_step, this->sbitmap[item].w);
          }
          else if (item + 1 == this->icon)
          {
            item_pos += is_tick(this->config_->scroll_step, this->sbitmap[item].w);
          }
          else if (this->icon == 3 && item == 1)
          {
            item_pos = target;
          }
        }
      }
      else if ((this->progress == 1) && this->icon == 1)
      {
        item_pos = target;
      }
      else
      {
        uint8_t reverse_steps = round(((static_cast<float>(width) - 8 * static_cast<float>(this->icon)) / static_cast<float>(this->icon + 1)) * this->icon + 8 * (this->icon + 1)) + 8;

        if (ceil((this->config_->next_action_time - this->config_->get_tick()) / EHMTXv2_SCROLL_INTERVAL) > reverse_steps)
        {
          if (item_pos > target)
          {
            item_pos = result > target ? result : target;
          }
        }
        else
        {
          item_pos -= is_tick(this->config_->scroll_step, this->sbitmap[item].w);
        }
      }

      c16to8(item_pos, this->sbitmap[item].r, this->sbitmap[item].g);
      result = item_pos;
    }

    return result;
  }

  int32_t EHMTX_queue::ypos()
  {
#ifdef EHMTXv2_USE_VERTICAL_SCROLL
    if (this->config_->queue_count() <= 1)
    {
      return 0;
    }

    uint8_t height = 8;

    if (ceil((this->config_->next_action_time - this->config_->get_tick()) / EHMTXv2_SCROLL_INTERVAL) > height)
    {
      if (this->config_->vertical_scroll)
      {
        return 0;
      }
      this->config_->vertical_scroll = this->config_->scroll_step >= height;
      return this->config_->scroll_step - height;
    }
    return height - round((this->config_->next_action_time - this->config_->get_tick()) / EHMTXv2_SCROLL_INTERVAL);
#else
    return 0;
#endif
  }

  int32_t EHMTX_queue::ypos(uint8_t item)
  {
    uint8_t height = 8;

    if ((this->progress == 1) && (this->icon == 1 || (this->icon == 3 && item == 1)))
    {
      if (ceil((this->config_->next_action_time - this->config_->get_tick()) / EHMTXv2_SCROLL_INTERVAL) > height)
      {
        if (this->default_font)
        {
          return 0;
        }
        this->default_font = this->config_->scroll_step >= height;
        return this->config_->scroll_step - height;
      }
      return height - round((this->config_->next_action_time - this->config_->get_tick()) / EHMTXv2_SCROLL_INTERVAL);
    }

    return 0;
  }

  void EHMTX_queue::update_screen()
  {
  if (this->icon < this->config_->icon_count)
    {
      if (millis() - this->config_->last_anim_time >= this->config_->icons[this->icon]->frame_duration)
      {
        this->config_->icons[this->icon]->next_frame();
        this->config_->last_anim_time = millis();
      }
    }
  }

  void EHMTX_queue::draw()
  {
    display::BaseFont *font = this->default_font ? this->config_->default_font : this->config_->special_font;
    display::BaseFont *info_font = this->config_->info_font ? this->config_->default_font : this->config_->special_font;

    int8_t yoffset = this->default_font ? EHMTXv2_DEFAULT_FONT_OFFSET_Y : EHMTXv2_SPECIAL_FONT_OFFSET_Y;
    int8_t xoffset = this->default_font ? EHMTXv2_DEFAULT_FONT_OFFSET_X : EHMTXv2_SPECIAL_FONT_OFFSET_X;

    Color color_;

    if (this->config_->is_running)
    {
      this->config_->display->clear();
      switch (this->mode)
      {
      case MODE_BLANK:
        break;

      case MODE_COLOR:
        this->config_->display->fill(this->text_color);
        break;

      case MODE_FULL_SCREEN:
        this->config_->display->image(0, this->ypos(), this->config_->icons[this->icon]);
        break;

      case MODE_ICON_DATE_TIME:
        if (this->config_->clock->now().is_valid())
        {
          color_ = this->text_color;
          
          // 1. Ikon kirajzolása (Bal szél: x=0)
          if (this->icon != BLANKICON) {
             this->config_->display->image(0, this->ypos(), this->config_->icons[this->icon]);
          }
          
          // 2. Dátum kirajzolása (Az ikon után)
          // Balra igazítjuk, kb. a 8. pixeltől, hogy elférjen az ikon után.
          this->config_->display->strftime(8 + xoffset, this->ypos() + yoffset, font, color_, display::TextAlign::BASELINE_LEFT, EHMTXv2_DATE_FORMAT, this->config_->clock->now());

          // 3. Idő kirajzolása (Jobb szélre igazítva)
          // Jobbra igazítjuk a 58. pixelhez (így biztosan a jobb szélen lesz).
          this->config_->display->strftime(58 + xoffset, this->ypos() + yoffset, font, color_, display::TextAlign::BASELINE_RIGHT, EHMTXv2_TIME_FORMAT, this->config_->clock->now());

          // 4. HÉT NAPJAI (DOW) KIRAJZOLÁSA
          // Ez a függvény rajzolja ki a kis csíkokat alulra.
          // A "true" paraméter jelzi, hogy "small" módban vagyunk (van ikon), 
          // így eltolja a csíkokat, hogy ne lógjanak bele az ikonba.
          this->config_->draw_day_of_week(this->ypos(), true);
        }
        break;        

      case MODE_ICON_CLOCK:
      case MODE_ICON_DATE:
        if (this->config_->clock->now().is_valid()) // valid time
        {
          color_ = this->text_color;
          time_t ts = this->config_->clock->now().timestamp;
          uint8_t offset = (this->config_->icon_to_9 == 3) ? 21 : 20;

          if (this->mode == MODE_ICON_CLOCK)
          {
#ifdef EHMTXv2_ADV_CLOCK
            if (!this->config_->draw_clock(EHMTXv2_TIME_FORMAT, font, color_, xoffset + offset, this->ypos() + yoffset))
            {
#endif
              if (this->config_->replace_time_date_active) // check for replace active
              {
                std::string time_new = this->config_->clock->now().strftime(EHMTXv2_TIME_FORMAT).c_str();
                time_new = this->config_->replace_time_date(time_new);
                this->config_->display->printf(xoffset + offset, this->ypos() + yoffset, font, color_, display::TextAlign::BASELINE_CENTER, "%s", time_new.c_str());
              }
              else
              {
                this->config_->display->strftime(xoffset + offset, this->ypos() + yoffset, font, color_, display::TextAlign::BASELINE_CENTER, EHMTXv2_TIME_FORMAT,
                                                 this->config_->clock->now());
              }
#ifdef EHMTXv2_ADV_CLOCK
            }
#endif
          }
          else
          {
#ifdef EHMTXv2_ADV_CLOCK
            if (!this->config_->draw_date(EHMTXv2_DATE_FORMAT, font, color_, xoffset + offset, this->ypos() + yoffset))
            {
#endif
              if (this->config_->replace_time_date_active) // check for replace active
              {
                std::string time_new = this->config_->clock->now().strftime(EHMTXv2_DATE_FORMAT).c_str();
                time_new = this->config_->replace_time_date(time_new);
                this->config_->display->printf(xoffset + offset, this->ypos() + yoffset, font, color_, display::TextAlign::BASELINE_CENTER, "%s", time_new.c_str());
              }
              else
              {
                this->config_->display->strftime(xoffset + offset, this->ypos() + yoffset, font, color_, display::TextAlign::BASELINE_CENTER, EHMTXv2_DATE_FORMAT,
                                                 this->config_->clock->now());
              }
#ifdef EHMTXv2_ADV_CLOCK
            }
#endif
          }
          if (this->icon != BLANKICON)
          {
            if (this->icon == SOLIDICON)
            {
              if ((this->mode == MODE_ICON_CLOCK && this->config_->icon_to_9 == 1) ||
                  (this->mode == MODE_ICON_DATE && this->config_->icon_to_9 == 2) ||
                  (this->config_->icon_to_9 == 3))
              {
                this->config_->display->filled_rectangle(0, this->ypos(), 9, 8, this->config_->solid_color);
              }
              else
              {
                this->config_->display->filled_rectangle(0, this->ypos(), 8, 8, this->config_->solid_color);
              }
            }
            else if (this->icon == CALENDARICON)
            {
              if ((this->mode == MODE_ICON_CLOCK && this->config_->icon_to_9 == 1) ||
                  (this->mode == MODE_ICON_DATE && this->config_->icon_to_9 == 2) ||
                  (this->config_->icon_to_9 == 3))
              {
                this->config_->display->filled_rectangle(0, this->ypos(), 9, 8, Color(C_RED, C_GREEN, C_BLUE));
                this->config_->display->filled_rectangle(0, this->ypos(), 9, 2, this->config_->calendar_color);
              }
              else
              {
                this->config_->display->filled_rectangle(0, this->ypos(), 8, 8, Color(C_RED, C_GREEN, C_BLUE));
                this->config_->display->filled_rectangle(0, this->ypos(), 8, 2, this->config_->calendar_color);
              }
            }
            else
            {
              if ((this->mode == MODE_ICON_CLOCK && this->config_->icon_to_9 == 1) ||
                  (this->mode == MODE_ICON_DATE && this->config_->icon_to_9 == 2) ||
                  (this->config_->icon_to_9 == 3))
              {
                this->config_->display->image(1, this->ypos(), this->config_->icons[this->icon]);
              }
              this->config_->display->image(0, this->ypos(), this->config_->icons[this->icon]);
            }
          }
          this->config_->draw_day_of_week(this->ypos(), true);

          if (this->icon_name.find("day") != std::string::npos || this->icon_name.find("weekday") != std::string::npos)
          {
            int8_t i_y_offset = this->config_->info_y_offset;
            Color i_lcolor = this->config_->info_lcolor;
            Color i_rcolor = this->config_->info_rcolor;

#ifdef EHMTXv2_ADV_CLOCK
            if (this->mode == MODE_ICON_CLOCK)
            {
              i_y_offset = this->config_->info_clock_y_offset;
              i_lcolor = this->config_->info_clock_lcolor;
              i_rcolor = this->config_->info_clock_rcolor;
              info_font = this->config_->info_clock_font ? this->config_->default_font : this->config_->special_font;
            }
            else // if (this->mode == MODE_ICON_DATE)
            {
              i_y_offset = this->config_->info_date_y_offset;
              i_lcolor = this->config_->info_date_lcolor;
              i_rcolor = this->config_->info_date_rcolor;
              info_font = this->config_->info_date_font ? this->config_->default_font : this->config_->special_font;
            }
#endif

            int32_t mode = 0;
            std::size_t pos = icon_name.find("#");
            if (pos != std::string::npos)
            {
              std::string str_mode = icon_name.substr(pos + 1);
              if (str_mode.length())
              {
                mode = std::stoi(str_mode);
              }
            }

            uint8_t x_left = 0;  // Leftmost point
            uint8_t x_right = 9; // Rightmost point

            if (this->icon_name.rfind("day", 0) == 0)
            {
              uint8_t d = this->config_->clock->now().day_of_month;

              // The symbol consists of a visible part, and an empty area to the right with a width of one point.
              uint8_t l_width = this->config_->GetTextWidth(info_font, "%d", (int32_t) (d / 10 % 10));
              uint8_t r_width = this->config_->GetTextWidth(info_font, "%d", (int32_t) (d % 10));
              switch (mode)
              {
              // To the center
              case 1:
              // To the center, the left one is a pixel higher.
              case 3:
              // To the center, the right one is a pixel higher.
              case 4:
              // To the center, without leading 0
              case 5:
                x_left = (l_width < 5) ? 5 - l_width : 0;
                x_right = 4;
                break;
              // Left to center, Right to edge
              case 2:
                x_left = (l_width < 5) ? 5 - l_width : 0;
                x_right = x_right - r_width;
                break;
              // To the edges
              default:
                x_right = x_right - r_width;
                break;
              }
              if (mode == 5 && (d < 10))
              {
                if (this->config_->icon_to_9 == 3)
                {
                  x_right = 5 - (r_width - 1) / 2;
                }
                else
                {
                  x_right = 4 - (r_width - 1) / 2;
                }
                this->config_->display->printf(x_right, this->ypos() + yoffset + i_y_offset, info_font, i_rcolor, display::TextAlign::BASELINE_LEFT, "%d", d % 10);
              }
              else
              {
                this->config_->display->printf(x_left, this->ypos() + yoffset + i_y_offset - (mode != 3 ? 0 : 1), info_font, i_lcolor, display::TextAlign::BASELINE_LEFT, "%d", d / 10 % 10);
                this->config_->display->printf(x_right, this->ypos() + yoffset + i_y_offset - (mode != 4 ? 0 : 1), info_font, i_rcolor, display::TextAlign::BASELINE_LEFT, "%d", d % 10);
              }
            }
            else // if (this->icon_name.rfind("weekday", 0) == 0)
            {
              uint8_t wd = this->config_->clock->now().day_of_week;

              if (this->config_->weekday_char_count > 7)
              {
                std::string left = this->config_->GetWeekdayChar((wd - 1) * 2);
                std::string right = this->config_->GetWeekdayChar((wd - 1) * 2 + 1);

                // The symbol consists of a visible part, and an empty area to the right with a width of one point.
                uint8_t l_width = this->config_->GetTextWidth(info_font, "%s", left.c_str());
                uint8_t r_width = this->config_->GetTextWidth(info_font, "%s", right.c_str());

                switch (mode)
                {
                // To the center
                case 1:
                // To the center, the left one is a pixel higher.
                case 3:
                // To the center, the right one is a pixel higher.
                case 4:
                  x_left = (l_width < 5) ? 5 - l_width : 0;
                  x_right = 4;
                  break;
                // Left to center, Right to edge
                case 2:
                  x_left = (l_width < 5) ? 5 - l_width : 0;
                  x_right = x_right - r_width;
                  break;
                // To the edges
                default:
                  x_right = x_right - r_width;
                  break;
                }
                this->config_->display->printf(x_left, this->ypos() + yoffset + i_y_offset - (mode != 3 ? 0 : 1), info_font, i_lcolor, display::TextAlign::BASELINE_LEFT, "%s", left.c_str());
                this->config_->display->printf(x_right, this->ypos() + yoffset + i_y_offset - (mode != 4 ? 0 : 1), info_font, i_rcolor, display::TextAlign::BASELINE_LEFT, "%s", right.c_str());
              }
              else
              {
                std::string weekday = this->config_->GetWeekdayChar(wd - 1);

                // The symbol consists of a visible part, and an empty area to the right with a width of one point.
                uint8_t c_width = this->config_->GetTextWidth(info_font, "%s", weekday.c_str());
                if (this->config_->icon_to_9 == 3)
                {
                  x_left = 5 - (c_width - 1) / 2;
                }
                else
                {
                  x_left = 4 - (c_width - 1) / 2;
                }
                this->config_->display->printf(x_left, this->ypos() + yoffset + i_y_offset, info_font, i_lcolor, display::TextAlign::BASELINE_LEFT, "%s", weekday.c_str());
              }
            }
          }
        }
        else
        {
          this->config_->display->print(xoffset + 19, yoffset, font, this->config_->alarm_color, display::TextAlign::BASELINE_CENTER, "!t!");
        }
        break;

      case MODE_ICON_SCREEN:
      case MODE_ALERT_SCREEN:
      case MODE_RAINBOW_ALERT_SCREEN:
      case MODE_RAINBOW_ICON:
      case MODE_ICON_PROGRESS:
      case MODE_PROGNOSIS_SCREEN:
        this->config_->display->start_clipping(8,0,64,8);
        color_ = (this->mode == MODE_RAINBOW_ICON || this->mode == MODE_RAINBOW_ALERT_SCREEN) ? this->config_->rainbow_color : this->text_color;
#ifdef EHMTXv2_USE_RTL
        this->config_->display->print(this->xpos() + xoffset, this->ypos() + yoffset, font, color_, esphome::display::TextAlign::BASELINE_RIGHT,
                                      this->text.c_str());
#else
#ifdef EHMTXv2_RAINBOW_SHIMMER
        if (this->mode == MODE_RAINBOW_ICON || this->mode == MODE_RAINBOW_ALERT_SCREEN)
          this->config_->draw_rainbow_text(this->text, font, this->xpos() + xoffset, this->ypos() + yoffset);
        else
#endif
          this->config_->draw_text(this->text, font, color_, this->xpos() + xoffset, this->ypos() + yoffset);
#endif
        this->config_->display->end_clipping();

        if (this->mode == MODE_ICON_PROGRESS)
        {
          if (this->icon != BLANKICON)
          {
            this->config_->display->line(8, this->ypos(), 8, this->ypos() + 7, esphome::display::COLOR_OFF);
            if (this->icon == SOLIDICON)
            {
              this->config_->display->filled_rectangle(0, this->ypos(), 8, 8, this->config_->solid_color);
            }
            else if (this->icon == CALENDARICON)
            {
              this->config_->display->filled_rectangle(0, this->ypos(), 8, 8, Color(C_RED, C_GREEN, C_BLUE));
              this->config_->display->filled_rectangle(0, this->ypos(), 8, 2, this->config_->calendar_color);
            }
            else
            {
              this->config_->display->image(0, this->ypos(), this->config_->icons[this->icon]);
            }
          }

          this->config_->display->line(9, this->ypos() + 7, 63, this->ypos() + 7, this->progressbar_back_color);
          if (this->progress != 0)
          {
            if (this->progressbar_color == esphome::display::COLOR_OFF)
            {
              color_ = esphome::light::ESPHSVColor(this->progress * 96 / 100 + (this->progress < 0 ? 96 : 0), 255, 240).to_rgb();
            }
            else
            {
              color_ = this->progressbar_color;
            }
            this->config_->display->line(9, this->ypos() + 7, 9 + abs(this->progress) * 54 / 100, this->ypos() + 7, color_);
          }
        }
        else if (this->mode == MODE_PROGNOSIS_SCREEN)
        {
          if (this->icon != BLANKICON)
          {
            this->config_->display->line(8, this->ypos(), 8, this->ypos() + 7, esphome::display::COLOR_OFF);
            if (this->icon == SOLIDICON)
            {
              this->config_->display->filled_rectangle(0, this->ypos(), 8, 8, this->config_->solid_color);
            }
            else if (this->icon == CALENDARICON)
            {
              this->config_->display->filled_rectangle(0, this->ypos(), 8, 8, Color(C_RED, C_GREEN, C_BLUE));
              this->config_->display->filled_rectangle(0, this->ypos(), 8, 2, this->config_->calendar_color);
            }
            else
            {
              this->config_->display->image(0, this->ypos(), this->config_->icons[this->icon]);
            }
          }

          if (this->sbitmap != NULL)
          {
            for (uint8_t x = 0; x < 56; x++)
            {
              this->config_->display->draw_pixel_at(8 + x, this->ypos() + 7, this->sbitmap[x]);
            }
          }
        }
        else
        {
          #ifdef EHMTXv2_GAUGE
          if (this->config_->display_gauge)
          {
            if (this->icon != BLANKICON)
            {
              if (this->icon == SOLIDICON)
              {
                this->config_->display->filled_rectangle(2, this->ypos(), 8, 8, this->config_->solid_color);
              }
              else if (this->icon == CALENDARICON)
              {
                this->config_->display->filled_rectangle(2, this->ypos(), 8, 8, Color(C_RED, C_GREEN, C_BLUE));
                this->config_->display->filled_rectangle(2, this->ypos(), 8, 2, this->config_->calendar_color);
              }
              else
              {
                this->config_->display->image(2, this->ypos(), this->config_->icons[this->icon]);
              }
            }
            this->config_->display->line(10, this->ypos(), 10, this->ypos() + 7, esphome::display::COLOR_OFF);
          }
          else
          #endif
          {
            this->config_->display->line(8, this->ypos(), 8, this->ypos() + 7, esphome::display::COLOR_OFF);
            if (this->icon != BLANKICON)
            {
              if (this->icon == SOLIDICON)
              {
                this->config_->display->filled_rectangle(0, this->ypos(), 8, 8, this->config_->solid_color);
              }
              else if (this->icon == CALENDARICON)
              {
                this->config_->display->filled_rectangle(0, this->ypos(), 8, 8, Color(C_RED, C_GREEN, C_BLUE));
                this->config_->display->filled_rectangle(0, this->ypos(), 8, 2, this->config_->calendar_color);
              }
              else
              {
                this->config_->display->image(0, this->ypos(), this->config_->icons[this->icon]);
              }
            }
          }
        }
        
        break;

      case MODE_TEXT_PROGRESS:
        color_ = this->text_color;

        this->config_->display->print(0, this->ypos() + yoffset, font, color_, esphome::display::TextAlign::BASELINE_LEFT, this->icon_name.c_str());

        this->config_->display->line(0, this->ypos() + 7, 63, this->ypos() + 7, this->progressbar_back_color);
        if (this->progress != 0)
        {
          if (this->progressbar_color == esphome::display::COLOR_OFF)
          {
            color_ = esphome::light::ESPHSVColor(this->progress * 120 / 100 + (this->progress < 0 ? 120 : 0), 255, 240).to_rgb();
          }
          else
          {
            color_ = this->progressbar_color;
          }
          this->config_->display->line(0, 7, abs(this->progress) * 63 / 100, 7, color_);
        }

        if (this->icon == 0)
        {
          color_ = this->text_color;
        }
        this->config_->display->print(64, this->ypos() + yoffset, font, color_, esphome::display::TextAlign::BASELINE_RIGHT, this->text.c_str());
        break;

      case MODE_ICON_TEXT_SCREEN:
      case MODE_RAINBOW_ICON_TEXT_SCREEN:
        this->config_->display->start_clipping(0,0,64,8);
        color_ = (this->mode == MODE_RAINBOW_ICON_TEXT_SCREEN) ? this->config_->rainbow_color : this->text_color;
#ifdef EHMTXv2_USE_RTL
        this->config_->display->print(this->xpos() + xoffset, this->ypos() + yoffset, font, color_, esphome::display::TextAlign::BASELINE_RIGHT,
                                      this->text.c_str());
#else
#ifdef EHMTXv2_RAINBOW_SHIMMER
        if (this->mode == MODE_RAINBOW_ICON_TEXT_SCREEN)
          this->config_->draw_rainbow_text(this->text, font, this->xpos() + xoffset, this->ypos() + yoffset);
        else
#endif
          this->config_->draw_text(this->text, font, color_, this->xpos() + xoffset, this->ypos() + yoffset);
#endif
        this->config_->display->end_clipping();

        if (this->icon != BLANKICON)
        {
          int32_t x = 0;
          if (this->pixels_ > 55)
          {
            if (this->xpos() > 55)
            {
              x = 56 - this->xpos();
            }
            else
            {
              if (this->xpos() < 9)
              {
                x = this->xpos() - 9;
              }
            }
          }
          this->config_->display->line(x + 8, this->ypos(), x + 8, this->ypos() + 7, esphome::display::COLOR_OFF);
          if (this->icon == SOLIDICON)
          {
            this->config_->display->filled_rectangle(x, this->ypos(), 8, 8, this->config_->solid_color);
          }
          else if (this->icon == CALENDARICON)
          {
            this->config_->display->filled_rectangle(x, this->ypos(), 8, 8, Color(C_RED, C_GREEN, C_BLUE));
            this->config_->display->filled_rectangle(x, this->ypos(), 8, 2, this->config_->calendar_color);
          }
          else
          {
            this->config_->display->image(x, this->ypos(), this->config_->icons[this->icon]);
          }
        }
        break;

      case MODE_TEXT_SCREEN:
      case MODE_RAINBOW_TEXT:
        color_ = (this->mode == MODE_RAINBOW_TEXT) ? this->config_->rainbow_color : this->text_color;
#ifdef EHMTXv2_USE_RTL
        this->config_->display->print(this->xpos() + xoffset, this->ypos() + yoffset, font, color_, esphome::display::TextAlign::BASELINE_RIGHT,
                                      this->text.c_str());
#else
#ifdef EHMTXv2_RAINBOW_SHIMMER
        if (this->mode == MODE_RAINBOW_TEXT)
          this->config_->draw_rainbow_text(this->text, font, this->xpos() + xoffset, this->ypos() + yoffset);
        else
#endif
          this->config_->draw_text(this->text, font, color_, this->xpos() + xoffset, this->ypos() + yoffset);
#endif
        break;

      default:
        ESP_LOGD(TAG, "no screen to draw!");
        this->config_->next_action_time = 0.0;
        break;
      }
      this->update_screen();
    }
  }

  void EHMTX_queue::hold_slot(uint8_t _sec)
  {
    this->endtime += 1000.0 * _sec;
    ESP_LOGD(TAG, "hold for %d secs", _sec);
  }

  // TODO void EHMTX_queue::set_mode_icon()
  // Screen Text, Screen time in seconds
  void EHMTX_queue::calc_scroll_time(std::string text, uint16_t screen_time)
  {
    int x, y, w, h;
    float display_duration;
    float requested_time = 1000.0 * screen_time;

    uint8_t width = 64;
    uint8_t startx = 0;
    uint16_t max_steps = 0;

    std::string text_ = text;
#ifdef EHMTXv2_MULTICOLOR_TEXT
    std::regex color_re("(#[A-Fa-f0-9]{6})");
    text_ = std::regex_replace(text, color_re, "");
#endif

    if (this->default_font)
    {
      this->config_->display->get_text_bounds(0, 0, text_.c_str(), this->config_->default_font, display::TextAlign::LEFT, &x, &y, &w, &h);
    }
    else
    {
      this->config_->display->get_text_bounds(0, 0, text_.c_str(), this->config_->special_font, display::TextAlign::LEFT, &x, &y, &w, &h);
    }

    this->pixels_ = w;

    switch (this->mode)
    {
    case MODE_TEXT_SCREEN:
    case MODE_RAINBOW_TEXT:
#ifdef EHMTXv2_SCROLL_SMALL_TEXT
      max_steps = EHMTXv2_SCROLL_COUNT * (width - startx) + EHMTXv2_SCROLL_COUNT * this->pixels_;
      display_duration = static_cast<float>(max_steps * EHMTXv2_SCROLL_INTERVAL);
      this->screen_time_ = (display_duration > requested_time) ? display_duration : requested_time;
#else
      if (this->pixels_ < 64)
      {
        this->screen_time_ = requested_time;
      }
      else
      {
        max_steps = EHMTXv2_SCROLL_COUNT * (width - startx) + EHMTXv2_SCROLL_COUNT * this->pixels_;
        display_duration = static_cast<float>(max_steps * EHMTXv2_SCROLL_INTERVAL);
        this->screen_time_ = (display_duration > requested_time) ? display_duration : requested_time;
      }
#endif
      break;
    case MODE_RAINBOW_ICON:
    case MODE_ICON_SCREEN:
    case MODE_ALERT_SCREEN:
    case MODE_RAINBOW_ALERT_SCREEN:
    case MODE_ICON_PROGRESS:
    case MODE_PROGNOSIS_SCREEN:
      startx = 8;
      if (this->pixels_ < 55)
      {
        this->screen_time_ = requested_time;
      }
      else
      {
        max_steps = EHMTXv2_SCROLL_COUNT * (width - startx) + EHMTXv2_SCROLL_COUNT * this->pixels_;
        display_duration = static_cast<float>(max_steps * EHMTXv2_SCROLL_INTERVAL);
        this->screen_time_ = (display_duration > requested_time) ? display_duration : requested_time;
      }
      break;
    case MODE_ICON_TEXT_SCREEN:
    case MODE_RAINBOW_ICON_TEXT_SCREEN:
      if (this->pixels_ < 55)
      {
        this->screen_time_ = requested_time;
      }
      else
      {
        max_steps = EHMTXv2_SCROLL_COUNT * (width - startx) + EHMTXv2_SCROLL_COUNT * this->pixels_;
        display_duration = static_cast<float>(max_steps * EHMTXv2_SCROLL_INTERVAL);
        this->screen_time_ = (display_duration > requested_time) ? display_duration : requested_time;
      }
      break;
    default:
      break;
    }

    this->scroll_reset = (width - startx) + this->pixels_;

    ESP_LOGD(TAG, "calc_scroll_time: mode: %d text: \"%s\" pixels %d calculated: %.1f defined: %d max_steps: %d", this->mode, text.c_str(), this->pixels_, this->screen_time_ / 1000.0, screen_time, this->scroll_reset);
  }

  // Icons count, Screen time in seconds
  void EHMTX_queue::calc_scroll_time(uint8_t icon_count, uint16_t screen_time)
  {
    float display_duration;
    float requested_time = 1000.0 * screen_time;

    uint8_t width = 64;
    uint8_t startx = 0;
    uint16_t max_steps = 0;

    this->pixels_ = 9 * icon_count;

    if (this->pixels_ < 64)
    {
      this->screen_time_ = requested_time;
    }
    else
    {
      max_steps = EHMTXv2_SCROLL_COUNT * (width - startx) + EHMTXv2_SCROLL_COUNT * this->pixels_;
      display_duration = static_cast<float>(max_steps * EHMTXv2_SCROLL_INTERVAL);
      this->screen_time_ = (display_duration > requested_time) ? display_duration : requested_time;
    }

    this->scroll_reset = (width - startx) + this->pixels_;

    ESP_LOGD(TAG, "calc_scroll_time: mode: %d icons count: %d pixels %d calculated: %.1f defined: %d max_steps: %d", this->mode, icon_count, this->pixels_, this->screen_time_ / 1000.0, screen_time, this->scroll_reset);
  }
}
