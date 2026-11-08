#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

template <typename T>
class ColaConcurrente {
public:
    // Devuelve false si la cola ya fue cerrada.
    bool agregar(T elemento) {
        {
            std::lock_guard<std::mutex> bloqueo(mutex_);
            if (cerrada_) return false;
            elementos_.push(std::move(elemento));
        }
        disponible_.notify_one();
        return true;
    }

    // Espera elementos; devuelve nullopt si está cerrada y vacía.
    std::optional<T> extraer() {
        std::unique_lock<std::mutex> bloqueo(mutex_);
        disponible_.wait(bloqueo, [this] {
            return cerrada_ || !elementos_.empty();
        });

        if (elementos_.empty()) return std::nullopt;

        T elemento = std::move(elementos_.front());
        elementos_.pop();
        return elemento;
    }

    void cerrar() {
        {
            std::lock_guard<std::mutex> bloqueo(mutex_);
            cerrada_ = true;
        }
        disponible_.notify_all();
    }

    std::size_t cantidad() const {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        return elementos_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable disponible_;
    std::queue<T> elementos_;
    bool cerrada_ = false;
};

