#pragma once
#ifdef __EMSCRIPTEN__

#include <FL/Fl.H>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <unordered_map>
#include <utility>

namespace EmFlTimeout {
    using Key = std::pair<Fl_Timeout_Handler, void*>;
    struct KeyHash {
        size_t operator()(const Key& k) const {
            return std::hash<void*>()((void*)k.first) ^ (std::hash<void*>()(k.second) << 1);
        }
    };

    struct TimerEntry {
        long id;
        Fl_Timeout_Handler cb;
        void* data;
        bool active;
    };

    inline std::unordered_map<Key, TimerEntry*, KeyHash>& active_timers() {
        static std::unordered_map<Key, TimerEntry*, KeyHash> timers;
        return timers;
    }

    inline void remove(Fl_Timeout_Handler cb, void* data = nullptr) {
        auto& timers = active_timers();
        auto it = timers.find({cb, data});
        if (it != timers.end()) {
            emscripten_clear_timeout(it->second->id);
            delete it->second;
            timers.erase(it);
        }
    }

    inline void fire_cb(void* raw_entry) {
        auto* entry = static_cast<TimerEntry*>(raw_entry);
        Fl_Timeout_Handler cb = entry->cb;
        void* data = entry->data;

        // Marca como inativo antes de chamar cb (cb pode chamar repeat_timeout ou remove_timeout)
        entry->active = false;

        if (cb) {
            cb(data);

            // OTIMIZAÇÃO 1: Só chama Fl::flush() se algum widget realmente pediu redesenho!
            // if (Fl::damage()) {
            //     Fl::flush();
            // }
        }

        // OTIMIZAÇÃO 2: Se cb NÃO reagendou o timer via repeat_timeout, limpa da memória agora
        auto& timers = active_timers();
        auto it = timers.find({cb, data});
        if (it != timers.end() && !it->second->active) {
            delete it->second;
            timers.erase(it);
        }
    }

    inline void add(double t, Fl_Timeout_Handler cb, void* data = nullptr) {
        double ms = (t <= 0.0) ? 16.0 : (t * 1000.0);
        auto& timers = active_timers();
        auto it = timers.find({cb, data});

        if (it != timers.end()) {
            // Reutiliza a mesma estrutura sem fazer new/delete!
            if (it->second->active) {
                emscripten_clear_timeout(it->second->id);
            }
            it->second->active = true;
            it->second->id = emscripten_set_timeout(fire_cb, ms, it->second);
        } else {
            auto* entry = new TimerEntry{0, cb, data, true};
            timers[{cb, data}] = entry;
            entry->id = emscripten_set_timeout(fire_cb, ms, entry);
        }
    }

    inline void repeat(double t, Fl_Timeout_Handler cb, void* data = nullptr) {
        add(t, cb, data);
    }
}

#define add_timeout(...)    has_timeout(nullptr) ? (void)0 : EmFlTimeout::add(__VA_ARGS__)
#define repeat_timeout(...) has_timeout(nullptr) ? (void)0 : EmFlTimeout::repeat(__VA_ARGS__)
#define remove_timeout(...) has_timeout(nullptr) ? (void)0 : EmFlTimeout::remove(__VA_ARGS__)

#endif 
