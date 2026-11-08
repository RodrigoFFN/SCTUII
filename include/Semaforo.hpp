#pragma once

#include <condition_variable>
#include <mutex>

enum class Luz { Rojo, Verde };

class Semaforo {
public:
    void cambiarA(Luz nuevaLuz) {
        {
            std::lock_guard<std::mutex> bloqueo(mutex_);
            luz_ = nuevaLuz;
        }
        disponible_.notify_all();
    }

    void esperarVerde() {
        std::unique_lock<std::mutex> bloqueo(mutex_);
        disponible_.wait(bloqueo, [this] { return luz_ == Luz::Verde; });
    }

    Luz actual() const {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        return luz_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable disponible_;
    Luz luz_ = Luz::Rojo;
};

