#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

namespace crypto_small
{
    namespace detail
    {
        inline uint32_t rotr(uint32_t x, uint32_t n) noexcept
        {
            return (x >> n) | (x << (32u - n));
        }

        struct Sha256Context
        {
            uint32_t state[8];
            uint64_t bitCount = 0;
            uint8_t buffer[64];
            uint32_t bufferLength = 0;
        };

        inline void sha256Init(Sha256Context& ctx) noexcept
        {
            ctx.state[0] = 0x6a09e667u;
            ctx.state[1] = 0xbb67ae85u;
            ctx.state[2] = 0x3c6ef372u;
            ctx.state[3] = 0xa54ff53au;
            ctx.state[4] = 0x510e527fu;
            ctx.state[5] = 0x9b05688cu;
            ctx.state[6] = 0x1f83d9abu;
            ctx.state[7] = 0x5be0cd19u;
            ctx.bitCount = 0;
            ctx.bufferLength = 0;
        }

        inline void sha256ProcessBlock(Sha256Context& ctx, const uint8_t* block) noexcept
        {
            uint32_t w[64];

            for (int i = 0; i < 16; ++i)
            {
                w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
                       (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                       (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
                       (static_cast<uint32_t>(block[i * 4 + 3]));
            }

            for (int i = 16; i < 64; ++i)
            {
                const uint32_t s0 = detail::rotr(w[i - 15], 7) ^ detail::rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
                const uint32_t s1 = detail::rotr(w[i - 2], 17) ^ detail::rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            uint32_t a = ctx.state[0];
            uint32_t b = ctx.state[1];
            uint32_t c = ctx.state[2];
            uint32_t d = ctx.state[3];
            uint32_t e = ctx.state[4];
            uint32_t f = ctx.state[5];
            uint32_t g = ctx.state[6];
            uint32_t h = ctx.state[7];

            static const uint32_t k[64] =
            {
                0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
                0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
                0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
                0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
                0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
                0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
                0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
                0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
            };

            for (int i = 0; i < 64; ++i)
            {
                const uint32_t S1 = detail::rotr(e, 6) ^ detail::rotr(e, 11) ^ detail::rotr(e, 25);
                const uint32_t ch = (e & f) ^ ((~e) & g);
                const uint32_t temp1 = h + S1 + ch + k[i] + w[i];
                const uint32_t S0 = detail::rotr(a, 2) ^ detail::rotr(a, 13) ^ detail::rotr(a, 22);
                const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                const uint32_t temp2 = S0 + maj;

                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }

            ctx.state[0] += a;
            ctx.state[1] += b;
            ctx.state[2] += c;
            ctx.state[3] += d;
            ctx.state[4] += e;
            ctx.state[5] += f;
            ctx.state[6] += g;
            ctx.state[7] += h;
        }

        inline void sha256Update(Sha256Context& ctx, const uint8_t* data, size_t len) noexcept
        {
            if (len == 0)
                return;

            ctx.bitCount += static_cast<uint64_t>(len) * 8u;

            while (len > 0)
            {
                const size_t space = 64u - ctx.bufferLength;
                const size_t toCopy = len < space ? len : space;

                for (size_t i = 0; i < toCopy; ++i)
                    ctx.buffer[ctx.bufferLength + i] = data[i];

                ctx.bufferLength += static_cast<uint32_t>(toCopy);
                data += toCopy;
                len -= toCopy;

                if (ctx.bufferLength == 64u)
                {
                    sha256ProcessBlock(ctx, ctx.buffer);
                    ctx.bufferLength = 0;
                }
            }
        }

        inline void sha256Final(Sha256Context& ctx, uint8_t output[32]) noexcept
        {
            // Append the bit '1' to the message
            ctx.buffer[ctx.bufferLength++] = 0x80u;

            if (ctx.bufferLength > 56u)
            {
                while (ctx.bufferLength < 64u)
                    ctx.buffer[ctx.bufferLength++] = 0u;
                sha256ProcessBlock(ctx, ctx.buffer);
                ctx.bufferLength = 0;
            }

            while (ctx.bufferLength < 56u)
                ctx.buffer[ctx.bufferLength++] = 0u;

            const uint64_t totalBits = ctx.bitCount;
            for (int i = 0; i < 8; ++i)
                ctx.buffer[63 - i] = static_cast<uint8_t>((totalBits >> (i * 8)) & 0xffu);

            sha256ProcessBlock(ctx, ctx.buffer);
            ctx.bufferLength = 0;

            for (int i = 0; i < 8; ++i)
            {
                output[i * 4]     = static_cast<uint8_t>((ctx.state[i] >> 24) & 0xffu);
                output[i * 4 + 1] = static_cast<uint8_t>((ctx.state[i] >> 16) & 0xffu);
                output[i * 4 + 2] = static_cast<uint8_t>((ctx.state[i] >> 8) & 0xffu);
                output[i * 4 + 3] = static_cast<uint8_t>((ctx.state[i]) & 0xffu);
            }
        }
    }

    inline std::array<uint8_t, 32> sha256(const uint8_t* data, size_t len) noexcept
    {
        detail::Sha256Context ctx;
        detail::sha256Init(ctx);
        detail::sha256Update(ctx, data, len);
        std::array<uint8_t, 32> out{};
        detail::sha256Final(ctx, out.data());
        return out;
    }

    inline std::array<uint8_t, 32> hmac_sha256(const uint8_t* key, size_t keyLen, const uint8_t* msg, size_t msgLen) noexcept
    {
        constexpr size_t blockSize = 64u;
        uint8_t ipad[blockSize];
        uint8_t opad[blockSize];
        uint8_t keyBlock[blockSize];

        if (keyLen > blockSize)
        {
            auto hashedKey = sha256(key, keyLen);
            for (size_t i = 0; i < blockSize; ++i)
                keyBlock[i] = i < hashedKey.size() ? hashedKey[i] : 0u;
        }
        else
        {
            for (size_t i = 0; i < blockSize; ++i)
                keyBlock[i] = (i < keyLen) ? key[i] : 0u;
        }

        for (size_t i = 0; i < blockSize; ++i)
        {
            ipad[i] = static_cast<uint8_t>(keyBlock[i] ^ 0x36u);
            opad[i] = static_cast<uint8_t>(keyBlock[i] ^ 0x5cu);
        }

        detail::Sha256Context innerCtx;
        detail::sha256Init(innerCtx);
        detail::sha256Update(innerCtx, ipad, blockSize);
        detail::sha256Update(innerCtx, msg, msgLen);
        uint8_t innerHash[32];
        detail::sha256Final(innerCtx, innerHash);

        detail::Sha256Context outerCtx;
        detail::sha256Init(outerCtx);
        detail::sha256Update(outerCtx, opad, blockSize);
        detail::sha256Update(outerCtx, innerHash, sizeof(innerHash));
        std::array<uint8_t, 32> result{};
        detail::sha256Final(outerCtx, result.data());
        return result;
    }

    inline std::array<uint8_t, 32> hmac_sha256(const std::array<uint8_t, 32>& key, const uint8_t* msg, size_t msgLen) noexcept
    {
        return hmac_sha256(key.data(), key.size(), msg, msgLen);
    }
}
