declare var angular: any;

interface Account {
    name: string;
    encoded: string;
}

interface Scope {
    accountText: string;
    accounts: Account[];
    addAccount: () => void;
}

(function () {
    angular.module('lazyApp', []).controller('LazyController', ['$scope', Ctrl]);

    function Ctrl($scope: Scope) {
        function isValid(name: string) {
            return name.trim() != "";
        }

        var testAccounts = ["google", "reddit", "facebook"];

        $scope.accounts = testAccounts.map(a => { return { name: a, encoded: "" }; });
        $scope.accountText = "";

        $scope.addAccount = function () {
            var account = $scope.accountText.trim();
            $scope.accountText = '';

            if (isValid(account)) {
                $scope.accounts.push({ name: account, encoded: encode(account) });
            }
        };
    }
})();