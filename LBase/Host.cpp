#include <LBase/Host.h>
#include <LBase/NativeAPI.h>
#include <LBase/FFileIO.h>
#if LFL_Win32
#include <LBase/Win32/Consoles.h>
#include <LBase/LConsole.h>
#include <io.h>
#endif

using namespace leo;
using platform::FileOperationFailure;
using platform::ThrowFileOperationFailure;
using platform::string;
using platform::string_view;

#if LB_Hosted

namespace platform_ex
{

	Exception::Exception(std::error_code ec, const char* str, RecordLevel lv)
		: system_error(ec, Nonnull(str)),
		level(lv)
	{}
	Exception::Exception(std::error_code ec, string_view sv, RecordLevel lv)
		: system_error(ec, (Nonnull(sv.data()), std::string(sv))),
		level(lv)
	{}
	Exception::Exception(int ev, const std::error_category& ecat, const char* str,
		RecordLevel lv)
		: system_error(ev, ecat, Nonnull(str)),
		level(lv)
	{}
	Exception::Exception(int ev, const std::error_category& ecat, string_view sv,
		RecordLevel lv)
		: system_error(ev, ecat, (Nonnull(sv.data()), std::string(sv))),
		level(lv)
	{}
	ImplDeDtor(Exception)

	leo::RecordLevel Exception::GetLevel() const lnothrow
	{
		return level;
	}

#if LFL_Win32
		void
		HandleDelete::operator()(pointer h) const lnothrow
	{
		LFL_CallWin32F_Trace(CloseHandle, h);
	}
#endif

	string
		FetchCommandOutput(const char* cmd, size_t buf_size)
	{
		if (LB_UNLIKELY(buf_size == 0))
			throw std::invalid_argument("Zero buffer size found.");
		// TODO: Improve Win32 implementation?
		if (const auto fp = leo::unique_raw(platform::upopen(cmd, "r"),
			platform::upclose))
		{
			leo::setnbuf(fp.get());

			// TODO: Improve performance?
			const auto p_buf(make_unique_default_init<char[]>(buf_size));
			string res;

			for (size_t n; (n = std::fread(&p_buf[0], 1, buf_size, fp.get())) != 0; )
				res.append(&p_buf[0], n);
			return res;
		}
		ThrowFileOperationFailure("Failed opening pipe.");
	}


	leo::locked_ptr<CommandCache>
		LockCommandCache()
	{
		struct cmd_cache_tag
		{};
		static leo::mutex mtx;

		return{ &leo::parameterize_static_object<CommandCache, cmd_cache_tag>(),
			mtx };
	}

	const string&
		FetchCachedCommandResult(const string& cmd, size_t buf_size)
	{
		auto p_locked(LockCommandCache());
		auto& cache(Deref(p_locked));

		try
		{
			// TODO: Blocked. Use %string_view as argument using C++14 heterogeneous
			//	%find template.
			const auto i_entry(cache.find(cmd));

			return (i_entry != cache.cend() ? i_entry : (cache.emplace(cmd,
				LB_UNLIKELY(cmd.empty()) ? string()
				: FetchCommandOutput(cmd.c_str(), buf_size))).first)->second;
		}
		CatchExpr(FileOperationFailure& e,
			TraceDe(Err, "Command execution failed: %s", e.what()))
			return cache[string()];
	}


	pair<UniqueHandle, UniqueHandle>
		MakePipe()
	{
#	if LFL_Win32
		::HANDLE h_raw_read, h_raw_write;

		LFL_CallWin32F(CreatePipe, &h_raw_read, &h_raw_write, {}, 0);

		UniqueHandle h_read(h_raw_read), h_write(h_raw_write);

		LFL_CallWin32F(SetHandleInformation, h_write.get(), HANDLE_FLAG_INHERIT,
			HANDLE_FLAG_INHERIT);
		return{ std::move(h_read), std::move(h_write) };
//#	elif YCL_API_Has_unistd_h
//		int fds[2];
//
//		// TODO: Check whether '::socketpair' is available.
//		if (::pipe(fds) != 0)
//			ThrowFileOperationFailure("Failed getting file size.");
//
//		auto pr(make_pair(UniqueHandle(fds[0]), UniqueHandle(fds[1])));
//		auto check([](UniqueHandle& h, const char* msg) {
//			// NOTE: %O_NONBLOCK is initially cleared on ::pipe results.
//			//	See http://pubs.opengroup.org/onlinepubs/9699919799/.
//			if (!(h && h->SetNonblocking()))
//				ThrowFileOperationFailure(msg);
//		});
//
//		check(pr.first, "Failed making pipe for reading."),
//			check(pr.second, "Failed making pipe for writing.");
//		return pr;
#	else
#	error "Unsupported platform found."
#	endif
	}


#	if LFL_Win32
	string
		DecodeArg(const char* str)
	{
		return Windows::MBCSToMBCS(str, CP_ACP, CP_UTF8);
	}

	string
		EncodeArg(const char* str)
	{
		return Windows::MBCSToMBCS(str);
	}
#	endif


#if !LFL_Android
#	if LFL_Win32
	class TerminalData : private Windows::WConsole
	{
	public:
		TerminalData(int fd)
			: WConsole(::HANDLE(::_get_osfhandle(fd)))
		{}

		PDefH(bool, RestoreAttributes, )
			ImplRet(WConsole::RestoreAttributes(), true)

			PDefH(bool, UpdateForeColor, std::uint8_t c)
			ImplRet(WConsole::UpdateForeColor(c), true)
	};
#	else
	namespace
	{

		lconstexpr const int cmap[] = { 0, 4, 2, 6, 1, 5, 3, 7 };

	} //unnamed namespace

	class TerminalData : private noncopyable, private nonmovable
	{
	private:
		std::FILE* stream;

	public:
		TerminalData(std::FILE* fp)
			: stream(fp)
		{}
	private:
		bool
			ExecuteCommand(const string&) const;

	public:
		PDefH(bool, RestoreAttributes, ) lnothrow
			ImplRet(ExecuteCommand("tput sgr0"))

			PDefH(bool, UpdateForeColor, std::uint8_t c) lnothrow
			ImplRet(ExecuteCommand("tput setaf " + to_string(cmap[c & 7]))
				&& (c < leo::underlying(leo::Consoles::DarkGray)
					|| ExecuteCommand("tput bold")))
	};

	bool
		TerminalData::ExecuteCommand(const string& cmd) const
	{
		const auto& str(FetchCachedCommandString(cmd));

		if (str.empty())
			return{};
		std::fprintf(Nonnull(stream), "%s", str.c_str());
		return true;
	}
#	endif


	Terminal::Guard::~Guard()
	{
		if (terminal)
			FilterExceptions([this] {
			if (!LB_LIKELY(terminal.p_term->RestoreAttributes()))
				throw LoggedEvent("Restoring terminal attributes failed.");
		});
	}


	Terminal::Terminal(std::FILE* fp)
		: p_term([fp]()->TerminalData* {
#	if LFL_Win32
		const int fd(::_fileno(Nonnull(fp)));

		// NOTE: This is not necessary for Windows since it only determine
		//	whether the file descriptor is associated with a character device.
		//	However as a optimization, it is somewhat more efficient for some
		//	cases. See $2015-01 @ %Documentation::Workflow::Annual2015.
		if (::_isatty(fd))
			TryRet(new TerminalData(fd))
#	else
		// XXX: Performance?
		leo::setnbuf(fp);

		const int fd(fileno(Nonnull(fp)));

		if (::isatty(fd))
			TryRet(new TerminalData(fp))
#	endif
			CatchExpr(Exception& e,
				TraceDe(Informative, "Creating console failed: %s.", e.what()))
			return{};
	}())
	{}

	ImplDeDtor(Terminal)

		bool
		Terminal::RestoreAttributes()
	{
		return p_term ? p_term->RestoreAttributes() : false;
	}

	bool
		Terminal::UpdateForeColor(std::uint8_t c)
	{
		return p_term ? p_term->UpdateForeColor(c) : false;
	}

	bool
		UpdateForeColorByLevel(Terminal& term, RecordLevel lv)
	{
		if (term)
		{
			using namespace Consoles;
			static lconstexpr const RecordLevel
				lvs[]{ Err, Warning, Notice, Informative, Debug };
			static lconstexpr const Color
				colors[]{ Red, Yellow, Cyan, Magenta, DarkGreen };
			const auto i(std::lower_bound(leo::begin(lvs), leo::end(lvs), lv));

			if (i == leo::end(lvs))
				term.RestoreAttributes();
			else
				term.UpdateForeColor(colors[i - lvs]);
			return true;
		}
		return{};
	}
#endif

} // namespace platform_ex;

#endif