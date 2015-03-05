(function () {
    angular.module('lazyApp', []).controller('LazyController', ['$scope', Ctrl]);

    function Ctrl($scope) {
        $scope.accounts = [
        { name: 'learn angular' },
        { name: 'build an angular app' }];

        $scope.addAccount = function () {
            $scope.accounts.push({ name: $scope.accountText, encoded: encode($scope.accountText) });
            $scope.accountText = '';
        };
    }
})();