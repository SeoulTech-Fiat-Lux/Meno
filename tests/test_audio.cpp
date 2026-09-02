#include <meno/audio/Music.hpp>
#include <meno/audio/Sound.hpp>

#include <filesystem>
#include <optional>
#include <type_traits>

static_assert(!std::is_copy_constructible_v<meno::Sound>);
static_assert(!std::is_copy_assignable_v<meno::Sound>);
static_assert(std::is_nothrow_move_constructible_v<meno::Sound>);
static_assert(std::is_nothrow_move_assignable_v<meno::Sound>);

static_assert(!std::is_copy_constructible_v<meno::Music>);
static_assert(!std::is_copy_assignable_v<meno::Music>);
static_assert(std::is_nothrow_move_constructible_v<meno::Music>);
static_assert(std::is_nothrow_move_assignable_v<meno::Music>);

static_assert(std::is_same_v<decltype(meno::Sound::loadFromFile(std::filesystem::path{})),
                             std::optional<meno::Sound>>);
static_assert(std::is_same_v<decltype(meno::Music::openFromFile(std::filesystem::path{})),
                             std::optional<meno::Music>>);

int main() {
    // 오디오 장치가 없는 CI에서도 공개 API와 소유권 계약을 검사한다.
    return 0;
}
