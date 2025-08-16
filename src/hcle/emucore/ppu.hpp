#ifndef __CYNES_PPU__
#define __CYNES_PPU__

#include <cstdint>
#include <memory>

#include "utils.hpp"

enum class SpriteEvaluationStep
{
    LOAD_SECONDARY_OAM,
    INCREMENT_POINTER,
    IDLE
};

struct PPUState
{
    uint16_t current_x;
    uint16_t current_y;

    bool frame_ready;

    bool rendering_enabled;
    bool rendering_enabled_delayed;
    bool prevent_vertical_blank;

    bool control_increment_mode;
    bool control_foreground_table;
    bool control_background_table;
    bool control_foreground_large;
    bool control_interrupt_on_vertical_blank;

    bool mask_grayscale_mode;
    bool mask_render_background_left;
    bool mask_render_foreground_left;
    bool mask_render_background;
    bool mask_render_foreground;

    uint8_t mask_color_emphasize;

    bool status_sprite_overflow;
    bool status_sprite_zero_hit;
    bool status_vertical_blank;

    uint8_t clock_decays[3];
    uint8_t register_decay;

    bool latch_cycle;
    bool latch_address;

    uint16_t register_t_;
    uint16_t register_v;
    uint16_t delayed_register_v;

    uint8_t scroll_x;
    uint8_t delay_data_read_counter;
    uint8_t delay_data_write_counter;
    uint8_t buffer_data;

    uint8_t background_data[0x4];
    uint16_t background_shifter[0x4];

    uint8_t foreground_data[0x20];
    uint8_t foreground_shifter[0x10];
    uint8_t foreground_attributes[0x8];
    uint8_t foreground_positions[0x8];

    uint8_t foreground_data_pointer;
    uint8_t foreground_sprite_count;
    uint8_t foreground_sprite_count_next;
    uint8_t foreground_sprite_pointer;
    uint8_t foreground_read_delay_counter;

    uint16_t foreground_sprite_address;

    bool foreground_sprite_zero_line;
    bool foreground_sprite_zero_should;
    bool foreground_sprite_zero_hit;

    SpriteEvaluationStep foreground_evaluation_step;
};

namespace cynes
{
    // Forward declaration.
    class NES;

    /// Picture Processing Unit (see https://www.nesdev.org/wiki/PPU).
    class PPU
    {
    public:
        /// Initialize the PPU.
        PPU(NES &nes);

        /// Default destructor.
        ~PPU() = default;

    public:
        /// Set the PPU in its power-up state.
        void power();

        /// Set the PPU in its reset state.
        void reset();

        /// Tick the PPU.
        void tick();

        /// Write to the PPU memory.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the PPU memory address space.
        /// @param value Value to write.
        void write(uint8_t address, uint8_t value);

        /// Read from the APU memory.
        /// @note This function has other side effects than simply reading from memory, it
        /// should not be used as a memory watch function.
        /// @param address Memory address within the PPU memory address space.
        /// @return The value stored at the given address.
        uint8_t read(uint8_t address);

        /// Get a pointer to the internal frame buffer.
        const uint8_t *get_frame_buffer() const;

        /// Check whether or not the frame is ready.
        /// @note Calling this function will reset the flag.
        /// @return True if the frame is ready, false otherwise.
        bool is_frame_ready();

        void setOutputModeGrayscale();

    private:
        NES &_nes;
        PPUState _state;

    private:
        std::unique_ptr<uint8_t[]> _frame_buffer;

        // === GRAYSCALE OUTPUT MODIFICATIONS ===
        using RenderPixelFunc = void (PPU::*)(size_t, uint8_t);
        RenderPixelFunc _render_pixel = &PPU::render_pixel_rgb;

        void render_pixel_rgb(size_t pixel_offset, uint8_t color_index);
        void render_pixel_gray(size_t pixel_offset, uint8_t color_index);

        uint8_t _palette_cache[32];
        void update_palette_cache();

    private:
        const uint8_t DECAY_PERIOD = 30;

    private:
        void increment_scroll_x();
        void increment_scroll_y();

        void reset_scroll_x();
        void reset_scroll_y();

    private:
        void load_background_shifters();
        void update_background_shifters();

    private:
        void reset_foreground_data();
        void clear_foreground_data();
        void fetch_foreground_data();
        void load_foreground_shifter();
        void update_foreground_shifter();

        uint8_t blend_colors();

    private:
        enum class Register : uint8_t
        {
            PPU_CTRL = 0x00,
            PPU_MASK = 0x01,
            PPU_STATUS = 0x02,
            OAM_ADDR = 0x03,
            OAM_DATA = 0x04,
            PPU_SCROLL = 0x05,
            PPU_ADDR = 0x06,
            PPU_DATA = 0x07
        };

    public:
        template <DumpOperation operation, typename T>
        constexpr void dump(T &buffer)
        {
            cynes::dump<operation>(buffer, _state);
        }
    };
}

#endif
