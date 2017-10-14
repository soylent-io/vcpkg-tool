#include "pch.h"

#include <vcpkg/base/sortedvector.h>
#include <vcpkg/base/system.h>
#include <vcpkg/commands.h>
#include <vcpkg/help.h>
#include <vcpkg/paragraphs.h>
#include <vcpkg/vcpkglib.h>

namespace vcpkg::Commands::Autocomplete
{
    std::vector<std::string> autocomplete_install(
        const std::vector<std::unique_ptr<SourceControlFile>>& source_paragraphs, const std::string& start_with)
    {
        std::vector<std::string> results;
        const auto& istartswith = Strings::case_insensitive_ascii_starts_with;

        for (const auto& source_control_file : source_paragraphs)
        {
            auto&& sp = *source_control_file->core_paragraph;

            if (istartswith(sp.name, start_with))
            {
                results.push_back(sp.name);
            }
        }
        return results;
    }

    std::vector<std::string> autocomplete_remove(std::vector<StatusParagraph*> installed_packages,
                                                 const std::string& start_with)
    {
        std::vector<std::string> results;
        const auto& istartswith = Strings::case_insensitive_ascii_starts_with;

        for (const auto& installed_package : installed_packages)
        {
            auto sp = installed_package->package.displayname();

            if (istartswith(sp, start_with))
            {
                results.push_back(sp);
            }
        }
        return results;
    }

    void perform_and_exit(const VcpkgCmdArguments& args, const VcpkgPaths& paths)
    {
        static const std::string EXAMPLE = Strings::format("The argument should be a command line to autocomplete.\n%s",
                                                           Help::create_example_string("autocomplete install z"));

        args.check_min_arg_count(1, EXAMPLE);
        args.check_max_arg_count(2, EXAMPLE);
        args.check_and_get_optional_command_arguments({});

        const std::string requested_command = args.command_arguments.at(0);
        const std::string start_with =
            args.command_arguments.size() > 1 ? args.command_arguments.at(1) : Strings::EMPTY;
        std::vector<std::string> results;
        if (requested_command == "install")
        {
            auto sources_and_errors = Paragraphs::try_load_all_ports(paths.get_filesystem(), paths.ports);
            auto& source_paragraphs = sources_and_errors.paragraphs;

            results = autocomplete_install(source_paragraphs, start_with);
        }
        else if (requested_command == "remove")
        {
            const StatusParagraphs status_db = database_load_check(paths);
            std::vector<StatusParagraph*> installed_packages = get_installed_ports(status_db);
            results = autocomplete_remove(installed_packages, start_with);
        }

        System::println(Strings::join(" ", results));
        Checks::exit_success(VCPKG_LINE_INFO);
    }
}