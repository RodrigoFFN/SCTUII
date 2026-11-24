#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>

// Modelo de animacion independiente de Qt. Las posiciones son esquematicas;
// la llegada, la entrada y la salida provienen de eventos reales del motor.
struct VehiculoAnimado {
    int id = 0;
    bool norteSur = true;
    int llegadaMs = 0;
    int entradaMs = -1;
    int salidaMs = -1;

    bool enCola() const { return entradaMs < 0 && salidaMs < 0; }
    bool esBus() const { return id % 3 == 0; }
};

class TrafficAnimationModel {
public:
    void clear() { coches_.clear(); }

    void registrar(const std::string& tipo, bool norteSur, int id, int cuandoMs,
                   int cruceMs = 130) {
        if (id <= 0) return;
        auto it = coches_.find(id);
        if (it == coches_.end()) {
            VehiculoAnimado v;
            v.id = id;
            v.norteSur = norteSur;
            v.llegadaMs = cuandoMs;
            it = coches_.emplace(id, v).first;
        }
        auto& v = it->second;
        if (tipo == "llegada") {
            v.llegadaMs = std::min(v.llegadaMs, cuandoMs);
        } else if (tipo == "cruce_inicio") {
            if (v.entradaMs < 0) v.entradaMs = cuandoMs;
        } else if (tipo == "paso") {
            if (v.entradaMs < 0) {
                // Compatible con monitores anteriores sin evento de entrada.
                v.entradaMs = std::max(v.llegadaMs, cuandoMs - cruceMs);
            }
            if (v.salidaMs < 0) v.salidaMs = cuandoMs;
        }
    }

    std::vector<VehiculoAnimado> visibles(int ahoraMs) const {
        std::vector<VehiculoAnimado> resultado;
        for (const auto& [id, coche] : coches_) {
            (void)id;
            if (coche.llegadaMs > ahoraMs) continue;
            if (coche.salidaMs >= 0 && ahoraMs - coche.salidaMs > 750) continue;
            resultado.push_back(coche);
        }
        return resultado;
    }

    int enCola(bool norteSur) const {
        int total = 0;
        for (const auto& [id, coche] : coches_) {
            (void)id;
            if (coche.norteSur == norteSur && coche.enCola()) ++total;
        }
        return total;
    }

    std::size_t totalRegistrados() const { return coches_.size(); }

private:
    std::map<int, VehiculoAnimado> coches_;
};
